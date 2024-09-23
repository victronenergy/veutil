#include <veutil/qt/ve_qitems_mqtt.hpp>

#include <QMutexLocker>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>

//--

VeQItemMqtt::VeQItemMqtt(VeQItemMqttProducer *producer)
	: VeQItem(producer)
{
	connect(producer, &VeQItemMqttProducer::connectionStateChanged,
		this, [this] {
			const VeQItemMqttProducer::ConnectionState state = mqttProducer()->connectionState();
			switch (state) {
			case VeQItemMqttProducer::Initializing:
				// No need to requestValue(), since we subscribe to everything
				// and send keepalive to read all values when Ready.
				// One possible issue: the VRM broker will provide retained
				// values (which might be stale/invalid for non-Settings paths).
				// To resolve this: drop any message from VRM broker with retained flag set.
				// Problem: QMqttClient doesn't expose the retain flag value...
				break;
			case VeQItemMqttProducer::Disconnected:
				produceValue(QVariant(), VeQItem::Offline);
				break;
			default: break;
			}
		});
}

int VeQItemMqtt::setValue(QVariant const &value)
{
	if (!mqttProducer() || !mqttProducer()->publishValue(uniqueId(), value)) {
		return -1;
	}
	return 0;
}

QVariant VeQItemMqtt::getValue()
{
	return getValue(false);
}

QVariant VeQItemMqtt::getValue(bool force)
{
	if (force && mqttProducer()) {
		mqttProducer()->requestValue(uniqueId());
	}
	return VeQItem::getValue(force);
}

// This is called by VeQItem::itemAddChild().
// TODO: may not be necessary, remove?
void VeQItemMqtt::setParent(QObject *parent)
{
	VeQItem::setParent(parent);
}

VeQItemMqttProducer *VeQItemMqtt::mqttProducer() const
{
	return static_cast<VeQItemMqttProducer*>(mProducer);
}

//--

VeQItemMqttProducer::VeQItemMqttProducer(
		VeQItem *root, const QString &id, const QString &clientIdPrefix, QObject *parent)
	: VeQItemProducer(root, id, parent),
	  mKeepAliveTimer(new QTimer(this)),
	  mHeartBeatTimer(new QTimer(this)),
	  mReadyStateFallbackTimer(new QTimer(this)),
	  mMqttConnection(nullptr),
	  mPort(0),
	  mHeartbeatState(HeartbeatInactive),
	  mConnectionState(Idle),
	  mAutoReconnectAttemptCounter(0),
	  mAutoReconnectMaxAttempts(sizeof(mReconnectAttemptIntervals)/sizeof(mReconnectAttemptIntervals[0])),
	  mError(QMqttClient::NoError),
	  mProtocolVersion(QMqttClient::MQTT_3_1_1),
	  mMissedHeartbeats(0),
	  mIsVrmBroker(false)
{
	// Create a sanitized clientId.  MQTT v3.1 spec states that the clientId must be
	// between 1 and 23 characters, and some brokers support [a-z][A-Z][0-9] only.
	for (const QChar &c : clientIdPrefix) {
		if (mClientId.size() < (23 - 16) &&
				((c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9'))) {
			mClientId.append(c);
		}
	}
	const quint64 uniqueId = QRandomGenerator::global()->generate64();
	mClientId.append(QStringLiteral("%1").arg(uniqueId, 16, 16, QLatin1Char('0')));

	// start the timer once we have sent the first (empty) keepalive after subscribing.
	mKeepAliveTimer->setInterval(1000 * 30);
	connect(mKeepAliveTimer, &QTimer::timeout,
		this, [this] {
			doKeepAlive(/* suppressRepublish = */ true);
		});

	// start the timer once we have received the first heartbeat.
	mHeartBeatTimer->setInterval(1000 * 5); // heartbeat interval is 3s, but allow extra.
	connect(mHeartBeatTimer, &QTimer::timeout,
		this, [this] {;
#ifdef MQTT_WEBSOCKETS_ENABLED
			if (mWebSocket && !mWebSocket->isOpen()) {
				qWarning() << "Missed heartbeat due to websocket disconnection";
				enqueueStateTransition({
					QStringLiteral("heartbeatTimer"),
					mConnectionState,
					Disconnected,
					true, // set error.
					QMqttClient::TransportInvalid
				});
				return;
			}
#endif
			++mMissedHeartbeats;
			qWarning() << "Missed" << mMissedHeartbeats << "heatbeat(s)!";
			if (mMissedHeartbeats == 2) {
				setHeartbeatState(HeartbeatMissing);
			} else if (mMissedHeartbeats > 2) {
				setHeartbeatState(HeartbeatInactive);
			}
		});

	mReadyStateFallbackTimer->setSingleShot(true);
	mReadyStateFallbackTimer->setInterval(5000);
	connect(mReadyStateFallbackTimer, &QTimer::timeout,
		this, [this] {
			if (mConnectionState == Initializing) {
				enqueueStateTransition({
					QStringLiteral("readyStateFallbackTimer"),
					Initializing,
					Ready,
					false, // don't set error.
					QMqttClient::NoError
				});
			}
		});
}

VeQItem *VeQItemMqttProducer::createItem()
{
	return new VeQItemMqtt(this);
}

bool VeQItemMqttProducer::isValidStateTransition(ConnectionState from, ConnectionState to) const
{
	switch (from) {
		case Idle:
			return to == WaitingToConnect;
		case WaitingToConnect:
			return to == TransportConnecting // WebSocket case
				|| to == Connecting
				|| to == Idle;
		case TransportConnecting:
			return to == TransportConnected
				|| to == Disconnected
				|| to == Idle;
		case TransportConnected:
			return to == Connecting
				|| to == Disconnected
				|| to == Idle;
		case Connecting:
			return to == Connected
				|| to == Disconnected
				|| to == Idle;
		case Connected:
			return to == Identified
				|| to == Disconnected
				|| to == Idle;
		case Identified:
			return to == Initializing
				|| to == Disconnected
				|| to == Idle;
		case Initializing:
			return to == Ready
				|| to == Disconnected
				|| to == Idle;
		case Ready:
			return to == Disconnected
				|| to == Idle;
		case Disconnected:
			return to == WaitingToConnect
				|| to == WaitingToReconnect
				|| to == Failed
				|| to == Idle;
		case WaitingToReconnect:
			return to == TransportReconnecting // WebSocket case
				|| to == Reconnecting
				|| to == Idle;
		case TransportReconnecting:
			return to == TransportReconnected
				|| to == Disconnected
				|| to == Idle;
		case TransportReconnected:
			return to == Reconnecting
				|| to == Disconnected
				|| to == Idle;
		case Reconnecting:
			return to == Connected
				|| to == Disconnected
				|| to == Idle;
		case Failed:
			return to == WaitingToConnect
				|| to == Idle;
		default: break;
	}
	return false;
}

void VeQItemMqttProducer::enqueueStateTransition(const StateTransition &transition)
{
	QMutexLocker locker(&mStateTransitionsMutex);
	mStateTransitions.enqueue(transition);
	if (mStateTransitions.size() == 1) {
		QMetaObject::invokeMethod(this, &VeQItemMqttProducer::transitionState, Qt::QueuedConnection);
	}
}

void VeQItemMqttProducer::transitionState()
{
	bool haveTransition = false;
	ConnectionState state = mConnectionState;
	StateTransition transition;
	transition.from = state;

	{
		QMutexLocker locker(&mStateTransitionsMutex);
		while (!mStateTransitions.isEmpty()) {
			StateTransition head = mStateTransitions.dequeue();
			// Due to various things being asynchronous (QIODevice, QMqttClient etc)
			// we cannot rely on all potential signal handlers being invoked in the
			// correct order (depending on whether they are direct or queued connections etc).
			// Thus, we need to ensure that we don't blindly transition between states.
			if (head.from == head.to && head.setError) {
				// Some "transitions" simply specify an error from lower levels.
				// If one of these is set, ensure that we set the error when we perform the transition,
				// but don't override a previously-set error (e.g. from Disconnect transition).
				if (!transition.setError || transition.error == QMqttClient::NoError) {
					transition.setError = true;
					transition.error = head.error;
				}
			} else if (head.from != state) {
				qWarning() << "Ignoring queued transition from old state:" << head.from << "to" << head.to << "by" << head.source << "as we are now in state:" << state;
			} else if (!isValidStateTransition(head.from, head.to)) {
				qWarning() << "Ignoring invalid transition from state:" << head.from << "to" << head.to << "by" << head.source;
			} else {
				// qDebug() << "Observing valid transition from state:" << head.from << "to" << head.to << "by" << head.source;
				// We have a valid transition.  Determine what sort of transition it is.
				haveTransition = true;
				if (head.to == Disconnected) {
					// Transitions to Disconnected state are higher priority than other transitions.
					transition.to = Disconnected;
					transition.source = head.source;
					// Such a transition may also have an error associated with it.
					// Allow overriding other previously-set errors with this one.
					if (head.setError && head.error != QMqttClient::NoError) {
						transition.setError = true;
						transition.error = head.error;
					}
				} else if (head.to == TransportConnected
						&& (state == Connecting || state == Reconnecting)
						&& transition.to == Connected) {
					// Transitions from Connecting to Connected are higher priority than
					// transitions from Connecting to TransportConnected.
					// So don't update the transition.to here.
				} else {
					transition.to = head.to;
					transition.source = head.source;
				}
			}
		}
	}

	if (transition.setError) {
		setError(transition.error);
	}

	if (haveTransition) {
		// transition to the appropriate state, then perform side effects
		// associated with that state.
		// qDebug() << "Transitioning from" << mConnectionState << " to " << transition.to;
		setConnectionState(transition.to);
		switch (transition.to) {
			case Idle: {
				stop();
				deleteMqttConnection();
				break;
			}

			case WaitingToConnect: {
				deleteMqttConnection();
				mMqttConnection = new QMqttClient(this);
				mMqttConnection->setClientId(mClientId);
				if (!mHostName.isEmpty()) {
					mMqttConnection->setHostname(mHostName);
					mMqttConnection->setPort(mPort);
				}

				connect(mMqttConnection, &QMqttClient::connected,
					this, &VeQItemMqttProducer::onConnected);
				connect(mMqttConnection, &QMqttClient::disconnected,
					this, &VeQItemMqttProducer::onDisconnected);
				connect(mMqttConnection, &QMqttClient::errorChanged,
					this, &VeQItemMqttProducer::onErrorChanged);
				connect(mMqttConnection, &QMqttClient::stateChanged,
					this, &VeQItemMqttProducer::onStateChanged);
				connect(mMqttConnection, &QMqttClient::messageReceived,
					this, &VeQItemMqttProducer::onMessageReceived);
			} // fall through
			case WaitingToReconnect: {
				const int interval = mReconnectAttemptIntervals[mAutoReconnectAttemptCounter++];
				QTimer::singleShot(
					interval + QRandomGenerator::global()->bounded(interval/2),
					this, [this] {
						enqueueStateTransition({
							mConnectionState == WaitingToConnect ? QStringLiteral("WaitingToConnect") : QStringLiteral("WaitingToReconnect"),
							mConnectionState,
#ifdef MQTT_WEBSOCKETS_ENABLED
							// Only transition to TransportConnecting state for WebSocket connections
							mHostName.isEmpty()
								? (mConnectionState == WaitingToConnect ? TransportConnecting : TransportReconnecting)
								: (mConnectionState == WaitingToConnect ? Connecting : Reconnecting),
#else
							mConnectionState == WaitingToConnect ? Connecting : Reconnecting,
#endif
							true, // clear any error from prior connection attempts.
							QMqttClient::NoError
						});
					});

				break;
			}

			case TransportConnecting: // fall through
			case TransportReconnecting: {
#ifdef MQTT_WEBSOCKETS_ENABLED
				// Note: the qDebug() seems to prevent a problem on Firefox which prevents reconnect.
				qDebug() << "TransportConnecting, MQTT connection is: " << mMqttConnection->state();
				deleteWebSocket();
				mWebSocket = new WebSocketDevice(this);
				mWebSocket->setUrl(mUrl);
				mWebSocket->setProtocol( mProtocolVersion == QMqttClient::MQTT_3_1
						? "mqttv3.1" : "mqtt");

				connect(mWebSocket, &WebSocketDevice::disconnected,
					this, &VeQItemMqttProducer::onSocketDisconnected);
				connect(mWebSocket, &WebSocketDevice::connected,
					this, &VeQItemMqttProducer::onSocketConnected);

				if (!mWebSocket->open(QIODeviceBase::ReadWrite)) {
					qWarning() << "Failed to open websocket to attempt to connect!";
					onSocketDisconnected(); // manually trigger failure codepath.
				}
#else
				Q_ASSERT_X(true, "MQTT producer", "Invalid transport connecting state in non-websockets build");
#endif
				break;
			}

			case TransportConnected: // fall through
			case TransportReconnected: {
#ifdef MQTT_WEBSOCKETS_ENABLED
				// Note: the qDebug() seems to prevent a problem on Firefox which prevents reconnect.
				qDebug() << "TransportConnected, MQTT connection is: " << mMqttConnection->state();
				// websocket has connected, now establish MQTT connection.
				mMqttConnection->setProtocolVersion(mProtocolVersion);
				setMqttConnectionTransport();
				enqueueStateTransition({
					mConnectionState == TransportConnected ? QStringLiteral("TransportConnected") : QStringLiteral("TransportReconnected"),
					mConnectionState,
					mConnectionState == TransportConnected ? Connecting : Reconnecting,
					false, // don't set error.
					QMqttClient::NoError
				});
#else
				Q_ASSERT_X(true, "MQTT producer", "Invalid transport connected state in non-websockets build");
#endif
				break;
			}

			case Connecting: // fall through
			case Reconnecting: {
				QMetaObject::invokeMethod(this, [this] { Q_EMIT aboutToConnect(); }, Qt::QueuedConnection);
				break;
			}

			case Connected: {
				mAutoReconnectAttemptCounter = 0;
				if (mPortalId.isEmpty()) {
					mMqttConnection->subscribe(QStringLiteral("N/+/system/0/Serial"));
				} else {
					// we already know the portal id, transition directly to the identified state.
					enqueueStateTransition({
						QStringLiteral("Connected"),
						Connected,
						Identified,
						false, // don't set error.
						QMqttClient::NoError
					});
				}
				break;
			}

			case Identified: {
				mMqttConnection->unsubscribe(QStringLiteral("N/+/system/0/Serial"));
				QObject::disconnect(mMqttConnection, &QMqttClient::messageReceived,
					this, &VeQItemMqttProducer::onMessageReceived);

				if (mMqttSubscription) {
					qWarning() << "MQTT: previous subscription was not cleared! FIXME!";
				}
				mMqttSubscription = mMqttConnection->subscribe(QStringLiteral("N/%1/#").arg(mPortalId));
				QObject::connect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
					this, &VeQItemMqttProducer::onSubscriptionMessageReceived, Qt::UniqueConnection);
				doKeepAlive(/* suppressRepublish = */ false);
				break;
			}

			case Initializing: {
				// In this state, we are waiting for the broker to send us
				// the full_publish_completed message.
				// We will transition to Ready state after some time
				// even if we are still receiving initial messages,
				// just in case the broker forgets to send us this message.
				mReadyStateFallbackTimer->start();
				break;
			}

			case Ready: {
				break;
			}

			case Disconnected: {
				stop();
#ifdef MQTT_WEBSOCKETS_ENABLED
				deleteWebSocket();
#endif
				if (mAutoReconnectAttemptCounter < mAutoReconnectMaxAttempts) {
					// Attempt to reconnect.  We use a staggered exponential backoff interval.
					enqueueStateTransition({
						QStringLiteral("Disconnected"),
						Disconnected,
						WaitingToReconnect,
						false, // don't set error.
						QMqttClient::NoError
					});
				} else {
					// Failed to connect.  After one minute we will attempt a clean connect.
					enqueueStateTransition({
						QStringLiteral("Disconnected"),
						Disconnected,
						Failed,
						false, // don't set error.
						QMqttClient::NoError
					});
				}
				break;
			}

			case Failed: {
				// Wait one minute and then start the connection process again.
				mAutoReconnectAttemptCounter = 0;
				QTimer::singleShot(
					60000,
					this, [this] {
						enqueueStateTransition({
							QStringLiteral("Failed"),
							Failed,
							WaitingToConnect,
							false, // don't set error.
							QMqttClient::NoError
						});
					});

				// On WebAssembly, the QMqttClient state can get
				// messed up.  Work around this.
				if (mMqttConnection) {
					stop();
					QTimer::singleShot(
						1000,
						this, [this] {
							qDebug() << "Deleting MQTT connection prior to retry in 60s";
							deleteMqttConnection();
						});
				}
				break;
			}

			default: {
				qWarning() << "Invalid destination state, ignoring.";
				break;
			}
		}
	}
}


#ifdef MQTT_WEBSOCKETS_ENABLED
void VeQItemMqttProducer::open(
		const QUrl &url,
		QMqttClient::ProtocolVersion protocolVersion)
{
	mIsVrmBroker = url.toString().startsWith(QStringLiteral("wss://webmqtt"))
			&& url.toString().contains(QStringLiteral(".victronenergy.com")); // might end with /mqtt?idUser=abcd

	if (mIsVrmBroker && mPortalId.isEmpty()) {
		// For VRM, need a portal id, as we don't want to subscribe to wildcard topics.
		qWarning() << "Attempting to connect to VRM broker, but no portal id provided!";
		enqueueStateTransition({
			QStringLiteral("open(url, protocolVersion)"),
			mConnectionState,
			Idle,
			true, // set error.
			QMqttClient::UnknownError
		});
		return;
	}

	mUrl = url;
	mProtocolVersion = protocolVersion;
	mHostName = QString();
	mPort = 0;

	enqueueStateTransition({
		QStringLiteral("open(url, protocolVersion)"),
		mConnectionState,
		WaitingToConnect,
		false, // don't set error.
		QMqttClient::NoError
	});
}
#endif // MQTT_WEBSOCKETS_ENABLED

void VeQItemMqttProducer::open(const QHostAddress &host, int port)
{
	mHostName = host.toString();
	mPort = port;
	mUrl = QUrl();

	enqueueStateTransition({
		QStringLiteral("open(host, port)"),
		mConnectionState,
		WaitingToConnect,
		false, // don't set error.
		QMqttClient::NoError
	});
}

void VeQItemMqttProducer::close()
{
	enqueueStateTransition({
		QStringLiteral("close"),
		mConnectionState,
		Idle,
		false, // don't set error.
		QMqttClient::NoError
	});
}

// Clients should call this method in their aboutToConnect() handler,
// prior to calling continueConnect().  This is because the VRM token may
// change and so during reconnect, they will need to update the credentials.
void VeQItemMqttProducer::setCredentials(const QString &username, const QString &password)
{
	if (mMqttConnection) {
		mMqttConnection->setUsername(username);
		mMqttConnection->setPassword(password);
	}
}

void VeQItemMqttProducer::continueConnect()
{
	// Use a queued connection to ensure that any client previously
	// deleteLater()'d will have been properly destroyed, to avoid
	// race condition where we might have two connections active.
	QMetaObject::invokeMethod(this, [this] {
		if (mMqttConnection) {
			mMqttConnection->setCleanSession(false);
			mMqttConnection->connectToHost();
		} else {
			// the MQTT connection was deleted out from under us...
			enqueueStateTransition({
				QStringLiteral("continueConnect"),
				mConnectionState,
				Failed,
				true, // set error.
				QMqttClient::UnknownError
			});
		}
	}, Qt::QueuedConnection);
}

void VeQItemMqttProducer::stop()
{
	setHeartbeatState(HeartbeatInactive);
	mMissedHeartbeats = 0;
	mKeepAliveTimer->stop();
	mHeartBeatTimer->stop();
	mReadyStateFallbackTimer->stop();
	if (mMqttSubscription.data()) {
		mMqttSubscription->unsubscribe();
		QObject::disconnect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
			this, &VeQItemMqttProducer::onSubscriptionMessageReceived);
		mMqttSubscription.clear();
	}
}

void VeQItemMqttProducer::onConnected()
{
	enqueueStateTransition({
		QStringLiteral("onConnected"),
		mConnectionState,
		Connected,
		false, // don't set error.
		QMqttClient::NoError
	});
}

void VeQItemMqttProducer::onDisconnected()
{
	enqueueStateTransition({
		QStringLiteral("onDisconnected"),
		mConnectionState,
		Disconnected,
		false, // don't set error.
		QMqttClient::NoError
	});
}

void VeQItemMqttProducer::onErrorChanged(QMqttClient::ClientError error)
{
	setError(error);
}

void VeQItemMqttProducer::onStateChanged(QMqttClient::ClientState state)
{
	// nothing to do.  we only care about connected/disconnected which is handled separately.
}

void VeQItemMqttProducer::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
	const QString topicName(topic.name());
	if (mPortalId.isEmpty()) {
		// the portal id should be reported via a message on topic: N/<portal id>/system/0/Serial
		if (topicName.endsWith(QStringLiteral("/system/0/Serial"))
				&& message.size()) {
			const QStringList parts = topicName.split('/');
			const QJsonObject payload = QJsonDocument::fromJson(message).object();
			if (parts.length() == 5 && parts[1] == payload.value(QStringLiteral("value")).toString()) {
				setPortalId(parts[1]);
				enqueueStateTransition({
					QStringLiteral("onMessageReceived"),
					mConnectionState,
					Identified,
					false, // don't set error.
					QMqttClient::NoError
				});
			} else {
				qWarning() << "VeQItemMqttProducer::onMessageReceived(): portal id mismatch: "
					<< topicName << " -> " << QString::fromUtf8(message);
			}
		}
	}
}

void VeQItemMqttProducer::onSubscriptionMessageReceived(const QMqttMessage &message)
{
	// Once we have received a message, transition to Initializing state.
	// Remain in this state while we wait for the flurry of initial messages to end.
	// The broker should tell us (with "full_publish_completed" topic message) when
	// we can transition to "Ready" state.
	if (mConnectionState == Identified) {
		enqueueStateTransition({
			QStringLiteral("onSubscriptionMessageReceived"),
			Identified,
			Initializing,
			false, // don't set error.
			QMqttClient::NoError
		});
	}

	// since the state transition (to Initializing) is queued,
	// we need to handle the message in a queued handler also.
	QMetaObject::invokeMethod(this, [this, message] { handleMessage(message); }, Qt::QueuedConnection);
}

void VeQItemMqttProducer::handleMessage(const QMqttMessage &message)
{
	const QString topicName(message.topic().name());
	const QString notificationPrefix = QStringLiteral("N/%1").arg(mPortalId);
	if (topicName.startsWith(notificationPrefix)) {
		const QString keepaliveTopic = notificationPrefix + QStringLiteral("/keepalive");
		const QString heartbeatTopic = notificationPrefix + QStringLiteral("/heartbeat");
		const QString readyTopic = notificationPrefix + QStringLiteral("/full_publish_completed");
		if (topicName.compare(keepaliveTopic, Qt::CaseInsensitive) == 0) {
			// ignore keepalive topic.
		} else if (mConnectionState == Initializing && topicName.compare(readyTopic, Qt::CaseInsensitive) == 0) {
			const QJsonObject payload = QJsonDocument::fromJson(message.payload()).object();
			if (payload.value(QStringLiteral("full-publish-completed-echo")).toString() == mFullPublishCompletedEcho) {
				// The broker has finished sending us the initial messages.  Transition to Ready state.
				enqueueStateTransition({
					QStringLiteral("handleMessage"),
					Initializing,
					Ready,
					false, // don't set error.
					QMqttClient::NoError
				});
			}
		} else if (topicName.compare(heartbeatTopic, Qt::CaseInsensitive) == 0) {
			// (re)start our heartbeat timer.
			mHeartBeatTimer->start();
			mMissedHeartbeats = 0;
			setHeartbeatState(HeartbeatActive);
		} else {
			// we have a topic message which we need to expose via VeQItem.
			const QString path = topicName.mid(notificationPrefix.size() + 1);
			parseMessage(path, message.payload());
		}
	}
}

void VeQItemMqttProducer::parseMessage(const QString &path, const QByteArray &message)
{
	VeQItemMqtt *item = qobject_cast<VeQItemMqtt*>(mProducerRoot->itemGetOrCreate(path, true, true));

	// if the payload is null/empty, then the device has been removed from the system.
	// invalidate the value, setting the item's state to Offline.
	if (message.size() == 0) {
		Q_EMIT nullMessageReceived(path);
		item->produceValue(QVariant(), VeQItem::Offline);
	} else {
		// otherwise, update the value in the item.
		const QJsonObject payload = QJsonDocument::fromJson(message).object();
		const QVariant min = payload.value(QStringLiteral("min")).toVariant();
		if (!min.isNull() && min.isValid()) {
			item->itemProduceProperty("min", min);
		}
		const QVariant max = payload.value(QStringLiteral("max")).toVariant();
		if (!max.isNull() && max.isValid()) {
			item->itemProduceProperty("max", max);
		}
		const QVariant def = payload.value(QStringLiteral("default")).toVariant();
		if (!def.isNull() && def.isValid()) {
			item->itemProduceProperty("default", def);
		}
		const QVariant value = payload.value(QStringLiteral("value")).toVariant();
		item->produceValue(value.isNull() ? QVariant() : value, // work around QJsonValue always using std::nullptr_t even for literal null values.
				VeQItem::Synchronized); // ensure the value is marked as "seen".
		Q_EMIT messageReceived(path, value);
	}
}

// The initial keepAlive sent immediately after subscribing to "N/portalId/#"
// should request a full_publish_completed message when broker has sent all initial messages.
// Periodic keepAlive messages sent every 30 seconds thereafter should
// suppress republish of non-changed values.
void VeQItemMqttProducer::doKeepAlive(bool suppressRepublish)
{
	if (mMqttConnection
			&& mMqttConnection->state() == QMqttClient::Connected
			&& !mPortalId.isEmpty()) {
#ifdef MQTT_WEBSOCKETS_ENABLED
		// If connected to VRM, it might not forward heartbeats from the device.
		// With Qt 6.5.2 we can sometimes lose the websocket connection
		// without that triggering automatic reconnect, and we won't catch
		// it in the heartbeat timeout handler if we are not receiving heartbeats.
		// So, handle it here, instead.
		if (mWebSocket && !mWebSocket->isOpen()) {
			qWarning() << "Unable to send keepalive due to websocket disconnection";
			onDisconnected(); // manually trigger state change to disconnected state.
			return;
		}
#endif
		if (!suppressRepublish) {
			// initial keep-alive.
			// see the related topic N/<portal_id>/full_publish_completed
			const quint64 uniqueId = QRandomGenerator::global()->generate64();
			mFullPublishCompletedEcho = QStringLiteral("%1").arg(uniqueId, 16, 16, QLatin1Char('0'));
			mMqttConnection->publish(QMqttTopicName(QStringLiteral("R/%1/keepalive").arg(mPortalId)),
					QStringLiteral("{ \"keepalive-options\" : [ { \"full-publish-completed-echo\": \"%1\" } ] }")
							.arg(mFullPublishCompletedEcho).toUtf8());
			mKeepAliveTimer->start();
		} else {
			mMqttConnection->publish(QMqttTopicName(QStringLiteral("R/%1/keepalive").arg(mPortalId)),
					QByteArrayLiteral("{ \"keepalive-options\" : [\"suppress-republish\"] }"));
		}
	}
}

VeQItemMqttProducer::HeartbeatState VeQItemMqttProducer::heartbeatState() const
{
	return mHeartbeatState;
}

void VeQItemMqttProducer::setHeartbeatState(VeQItemMqttProducer::HeartbeatState heartbeatState)
{
	if (mHeartbeatState != heartbeatState) {
		mHeartbeatState = heartbeatState;
		Q_EMIT heartbeatStateChanged();
	}
}

VeQItemMqttProducer::ConnectionState VeQItemMqttProducer::connectionState() const
{
	return mConnectionState;
}

void VeQItemMqttProducer::setConnectionState(VeQItemMqttProducer::ConnectionState connectionState)
{
	if (mConnectionState != connectionState) {
		mConnectionState = connectionState;
		Q_EMIT connectionStateChanged();
	}
}

QMqttClient::ClientError VeQItemMqttProducer::error() const
{
	return mError;
}

void VeQItemMqttProducer::setError(QMqttClient::ClientError error)
{
	if (mError != error) {
		mError = error;
		Q_EMIT errorChanged();
	}
}

QString VeQItemMqttProducer::portalId() const
{
	return mPortalId;
}

void VeQItemMqttProducer::setPortalId(const QString &portalId)
{
	if (mPortalId != portalId) {
		mPortalId = portalId;
		Q_EMIT portalIdChanged();
	}
}

QMqttClient *VeQItemMqttProducer::mqttConnection() const
{
	return mMqttConnection;
}

bool VeQItemMqttProducer::publishValue(const QString &uid, const QVariant &value)
{
	if (!mMqttConnection) {
		return false;
	}

	const QString topic = QStringLiteral("W/%1/%2").arg(mPortalId, uid.mid(5));
	const QJsonObject obj { { QStringLiteral("value"), QJsonValue::fromVariant(value) } };
	const QJsonDocument doc(obj);
	mMqttConnection->publish(topic, doc.toJson(QJsonDocument::Compact));
	return true;
}

bool VeQItemMqttProducer::requestValue(const QString &uid)
{
	if (!mMqttConnection) {
		return false;
	}

	const QString topic = QStringLiteral("R/%1/%2").arg(mPortalId, uid.mid(5));
	mMqttConnection->publish(topic, QByteArray());
	return true;
}

void VeQItemMqttProducer::deleteMqttConnection()
{
	if (mMqttSubscription.data()) {
		mMqttSubscription->unsubscribe();
		QObject::disconnect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
			this, &VeQItemMqttProducer::onSubscriptionMessageReceived);
		mMqttSubscription.clear();
	}
	if (mMqttConnection) {
		disconnect(mMqttConnection, nullptr, this, nullptr);
		mMqttConnection->disconnectFromHost();
		mMqttConnection->deleteLater();
		mMqttConnection = nullptr;
	}
#ifdef MQTT_WEBSOCKETS_ENABLED
	if (mCleanupWebSocket) {
		mCleanupWebSocket->deleteLater();
		mCleanupWebSocket = nullptr;
	}
	if (mWebSocket) {
		disconnect(mWebSocket, nullptr, this, nullptr);
		mWebSocket->close();
		mWebSocket->deleteLater();
		mWebSocket = nullptr;
	}
#endif
}

#ifdef MQTT_WEBSOCKETS_ENABLED
void VeQItemMqttProducer::onSocketConnected()
{
	enqueueStateTransition({
			QStringLiteral("onSocketConnected"),
			mConnectionState,
			mConnectionState == TransportConnecting ? TransportConnected : TransportReconnected,
			false, // don't set error.
			QMqttClient::NoError
		});
}

void VeQItemMqttProducer::onSocketDisconnected()
{
	// In Connected state there is nothing to do:
	// the QMqttClient should react to the QIODevice::aboutToClose().
	// But, if we receive this signal during Connecting state,
	// we need to manually handle the state transition,
	// as it means that the websocket was unable to connect.
	if (mConnectionState == TransportConnecting
			|| mConnectionState == TransportReconnecting) {
		enqueueStateTransition({
				QStringLiteral("onSocketDisconnected"),
				mConnectionState,
				Disconnected,
				true, // set error.
				QMqttClient::TransportInvalid
			});
	}
}

// QMqttClient doesn't properly guard all accesses to its transport,
// so we cannot set it to nullptr if we need to delete the WebSocket.
// Thus, we cannot fully delete an old WebSocket if we have set it
// as the transport for the QMqttClient, until we set the new transport.
// So, keep the old websocket pointer around and delete it when we can.
void VeQItemMqttProducer::setMqttConnectionTransport()
{
	mMqttConnection->setTransport(mWebSocket, QMqttClient::IODevice);
	if (mCleanupWebSocket) {
		mCleanupWebSocket->deleteLater();
		mCleanupWebSocket = nullptr;
	}
}
void VeQItemMqttProducer::deleteWebSocket()
{
	if (mWebSocket) {
		if (mCleanupWebSocket) {
			// the current mWebSocket hasn't yet been set as the
			// transport for the QMqttConnection, so we can
			// just delete it directly.
			disconnect(mWebSocket, nullptr, this, nullptr);
			mWebSocket->close();
			mWebSocket->deleteLater();
		} else {
			// the mWebSocket has been set as the transport.
			// we cannot delete it, until we set a new one
			// as the transport of the QMqttClient.
			mCleanupWebSocket = mWebSocket;
			disconnect(mCleanupWebSocket, nullptr, this, nullptr);
			mCleanupWebSocket->close();
		}
		mWebSocket = nullptr;
	}
}
#endif // MQTT_WEBSOCKETS_ENABLED

//--

#ifdef MQTT_WEBSOCKETS_ENABLED
#include <QtWebSockets/qwebsockethandshakeoptions.h>
WebSocketDevice::WebSocketDevice(QObject *parent)
	: QIODevice(parent)
{
	//connect(&mWebSocket, &QWebSocket::stateChanged,
	//	this, [this] {
	//		qDebug() << "WebSocket::onStateChanged: " << mWebSocket.state();
	//	});
	connect(&mWebSocket, &QWebSocket::connected,
		this, &WebSocketDevice::onSocketConnected);
	connect(&mWebSocket, &QWebSocket::disconnected,
		this, &WebSocketDevice::onSocketDisconnected);
	connect(&mWebSocket, &QWebSocket::binaryMessageReceived,
		this, &WebSocketDevice::onBinaryMessageReceived);
}

void WebSocketDevice::setUrl(const QUrl &url)
{
	if (mUrl != url) {
		mUrl = url;
		Q_EMIT urlChanged();
	}
}

QUrl WebSocketDevice::url() const
{
	return mUrl;
}

void WebSocketDevice::setProtocol(const QByteArray &protocol)
{
	if (mProtocol != protocol) {
		mProtocol = protocol;
		Q_EMIT protocolChanged();
	}
}

QByteArray WebSocketDevice::protocol() const
{
	return mProtocol;
}

bool WebSocketDevice::isValid() const
{
	return mWebSocket.isValid();
}

bool WebSocketDevice::open(QIODeviceBase::OpenMode mode)
{
	if (mState != QAbstractSocket::UnconnectedState) {
		return false;
	}

	QWebSocketHandshakeOptions options;
	options.setSubprotocols(QStringList { QString::fromUtf8(mProtocol) });
	mWebSocket.open(mUrl, options);
	const bool opening = QIODevice::open(mode);
	mState = opening ? QAbstractSocket::ConnectingState : QAbstractSocket::UnconnectedState;
	return opening;
}

void WebSocketDevice::close()
{
	mState = QAbstractSocket::ClosingState;
	if (mWebSocket.isValid()) {
		mWebSocket.close();
	}
	if (isOpen()) {
		QIODevice::close();
	}
	mState = QAbstractSocket::UnconnectedState;
}

qint64 WebSocketDevice::readData(char *data, qint64 maxSize)
{
	const qint64 count = maxSize < mData.size() ? maxSize : mData.size();
	memcpy(data, mData.constData(), count);
	mData = mData.right(mData.size() - count);
	return count;
}

qint64 WebSocketDevice::writeData(const char *data, qint64 maxSize)
{
	return mWebSocket.sendBinaryMessage(QByteArray(data, maxSize));
}

qint64 WebSocketDevice::bytesAvailable() const
{
	return mData.size() + QIODevice::bytesAvailable();
}

bool WebSocketDevice::isSequential() const
{
	return true;
}

void WebSocketDevice::onSocketConnected()
{
	if (mState != QAbstractSocket::ConnectedState) {
		mState = QAbstractSocket::ConnectedState;
		Q_EMIT connected();
	}
}

void WebSocketDevice::onSocketDisconnected()
{
	if (mState == QAbstractSocket::ConnectingState
			|| mState == QAbstractSocket::ConnectedState) {
		close();
		Q_EMIT disconnected();
	}
}

void WebSocketDevice::onBinaryMessageReceived(const QByteArray &message)
{
	mData.append(message);
	Q_EMIT readyRead();
}
#endif // MQTT_WEBSOCKETS_ENABLED

