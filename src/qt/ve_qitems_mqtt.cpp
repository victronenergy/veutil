#include <veutil/qt/ve_qitems_mqtt.hpp>

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
	  mReceivedMessage(false),
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
				onDisconnected();
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
			if (connectionState() == Initializing) {
				setConnectionState(Ready);
			}
		});
}

VeQItem *VeQItemMqttProducer::createItem()
{
	return new VeQItemMqtt(this);
}

#ifdef MQTT_WEBSOCKETS_ENABLED
void VeQItemMqttProducer::open(
		const QUrl &url,
		QMqttClient::ProtocolVersion protocolVersion)
{
	mIsVrmBroker = url.toString().startsWith(QStringLiteral("wss://webmqtt"))
			&& url.toString().endsWith(QStringLiteral(".victronenergy.com/mqtt"));
	mAutoReconnectAttemptCounter = 0;
	setError(QMqttClient::NoError);

	// Invoke via queued connection to ensure that the children
	// are created in the appropriate thread.
	QMetaObject::invokeMethod(this, [this, url, protocolVersion] {
		mUrl = url;
		mProtocolVersion = protocolVersion;
		mHostName = QString();
		mPort = 0;

		if (mMqttConnection) {
			mMqttConnection->deleteLater();
		}

		mMqttConnection = new QMqttClient(this);
		mMqttConnection->setClientId(mClientId);

		connect(mMqttConnection, &QMqttClient::connected,
			this, &VeQItemMqttProducer::onConnected);
		connect(mMqttConnection, &QMqttClient::disconnected,
			this, &VeQItemMqttProducer::onDisconnected);
		connect(mMqttConnection, &QMqttClient::errorChanged,
			this, &VeQItemMqttProducer::onErrorChanged);
		connect(mMqttConnection, &QMqttClient::stateChanged,
			this, &VeQItemMqttProducer::onStateChanged,
			Qt::QueuedConnection); // Queued to avoid double onDisconnected().
		connect(mMqttConnection, &QMqttClient::messageReceived,
			this, &VeQItemMqttProducer::onMessageReceived);

		if (mWebSocket) {
			mWebSocket->deleteLater();
		}

		mWebSocket = new WebSocketDevice(mMqttConnection);
		mWebSocket->setUrl(url);
		mWebSocket->setProtocol(
			  protocolVersion == QMqttClient::MQTT_3_1 ? "mqttv3.1" : "mqtt");

		connect(mWebSocket, &WebSocketDevice::disconnected,
			this, [this] {
				onStateChanged(QMqttClient::Disconnected);
			}, Qt::QueuedConnection);

		connect(mWebSocket, &WebSocketDevice::connected,
			this, [this, protocolVersion] {
				mMqttConnection->setProtocolVersion(protocolVersion);
				mMqttConnection->setTransport(mWebSocket, QMqttClient::IODevice);
				QMetaObject::invokeMethod(this, [this] { Q_EMIT aboutToConnect(); }, Qt::QueuedConnection);
			}, Qt::QueuedConnection);

		if (connectionState() != Reconnecting) {
			setConnectionState(Connecting);
		}
		mWebSocket->open(QIODeviceBase::ReadWrite);
	}, Qt::QueuedConnection);
}
#endif // MQTT_WEBSOCKETS_ENABLED

void VeQItemMqttProducer::open(const QHostAddress &host, int port)
{
	mAutoReconnectAttemptCounter = 0;
	setError(QMqttClient::NoError);

	// Invoke via queued connection to ensure that the children
	// are created in the appropriate thread.
	QMetaObject::invokeMethod(this, [this, host, port] {
		mHostName = host.toString();
		mPort = port;
		mUrl = QUrl();

		if (mMqttConnection) {
			mMqttConnection->deleteLater();
		}

		mMqttConnection = new QMqttClient(this);
		mMqttConnection->setClientId(mClientId);
		mMqttConnection->setHostname(mHostName);
		mMqttConnection->setPort(mPort);

		connect(mMqttConnection, &QMqttClient::connected,
			this, &VeQItemMqttProducer::onConnected);
		connect(mMqttConnection, &QMqttClient::disconnected,
			this, &VeQItemMqttProducer::onDisconnected);
		connect(mMqttConnection, &QMqttClient::errorChanged,
			this, &VeQItemMqttProducer::onErrorChanged);
		connect(mMqttConnection, &QMqttClient::stateChanged,
			this, &VeQItemMqttProducer::onStateChanged,
			Qt::QueuedConnection); // Queued to avoid double onDisconnected().
		connect(mMqttConnection, &QMqttClient::messageReceived,
			this, &VeQItemMqttProducer::onMessageReceived);

		QMetaObject::invokeMethod(this, [this] { Q_EMIT aboutToConnect(); }, Qt::QueuedConnection);
		if (connectionState() != Reconnecting) {
			setConnectionState(Connecting);
		}
	}, Qt::QueuedConnection);
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
		mMqttConnection->setCleanSession(false);
		mMqttConnection->connectToHost();
	}, Qt::QueuedConnection);
}

void VeQItemMqttProducer::onConnected()
{
	mAutoReconnectAttemptCounter = 0;
	setConnectionState(Connected);

	if (mPortalId.isEmpty()) {
		mMqttConnection->subscribe(QStringLiteral("N/+/system/0/Serial"));
	} else {
		QObject::disconnect(mMqttConnection, &QMqttClient::messageReceived,
			this, &VeQItemMqttProducer::onMessageReceived);
		mMqttSubscription = mMqttConnection->subscribe(QStringLiteral("N/%1/#").arg(mPortalId));
		QObject::connect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
			this, &VeQItemMqttProducer::onSubscriptionMessageReceived, Qt::UniqueConnection);
		doKeepAlive();
	}

	// QMqttClient has some internal state which sometimes isn't cleaned up
	// properly on disconnection.  To allow reconnection, we need to delete
	// the client, transition back to idle state, and then reconnect.
	QTimer::singleShot(3000, this, [this] {
		// If we're still waiting to receive a message from the broker more
		// than 3 seconds after connecting, assume we are in the stuck state.
		if (connectionState() == VeQItemMqttProducer::Connected) {
			qWarning() << "Automatic reconnection failed, trying clean reconnect.";
			mMqttConnection->deleteLater();
			mMqttConnection = nullptr;
			setConnectionState(Idle);
			// Immediately reopening the connection can fail.
			// Allow asynchronous cleanup to complete before reconnecting.
			QTimer::singleShot(1000, this, [this] {
#ifdef MQTT_WEBSOCKETS_ENABLED
				if (mHostName.isEmpty()) {
					open(mUrl, mProtocolVersion);
				} else {
					open(QHostAddress(mHostName), mPort);
				}
#else
				open(QHostAddress(mHostName), mPort);
#endif
			});
		}
	});
}

void VeQItemMqttProducer::onDisconnected()
{
	setHeartbeatState(HeartbeatInactive);
	setConnectionState(Disconnected);
	if (error() == QMqttClient::NoError
			&& mMqttConnection
			&& mMqttConnection->error() != QMqttClient::NoError) {
		setError(mMqttConnection->error());
	}
	mMissedHeartbeats = 0;
	mKeepAliveTimer->stop();
	mHeartBeatTimer->stop();
	mReadyStateFallbackTimer->stop();
	mReceivedMessage = false;
	if (mMqttSubscription.data()) {
		mMqttSubscription->unsubscribe();
		QObject::disconnect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
			this, &VeQItemMqttProducer::onSubscriptionMessageReceived);
	}

	if (mAutoReconnectAttemptCounter < mAutoReconnectMaxAttempts) {
		// Attempt to reconnect.  We use a staggered exponential backoff interval.
		const int interval = mReconnectAttemptIntervals[mAutoReconnectAttemptCounter++];
#ifdef MQTT_WEBSOCKETS_ENABLED
		if (!mWebSocket || !mWebSocket->isValid()) {
			QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
					this, [this] {
						setConnectionState(Reconnecting);
						quint16 count = mAutoReconnectAttemptCounter;
						if (mHostName.isEmpty()) {
							open(mUrl, mProtocolVersion);
						} else {
							open(QHostAddress(mHostName), mPort);
						}
						mAutoReconnectAttemptCounter = count;
					});
		} else {
			QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
					this, [this] {
						setConnectionState(Reconnecting);
						emit aboutToConnect();
					});
		}
#else
		QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
				this, [this] {
					setConnectionState(Reconnecting);
					emit aboutToConnect();
				});
#endif
	} else {
		// Failed to connect.  Wait one minute and then start the connection process again.
		setConnectionState(Failed);
		mAutoReconnectAttemptCounter = 0;
		QTimer::singleShot(60000, this, &VeQItemMqttProducer::onDisconnected);
	}
}

void VeQItemMqttProducer::onErrorChanged(QMqttClient::ClientError error)
{
	setError(error);

	if (mMqttConnection && mMqttConnection->state() == QMqttClient::Disconnected
			&& (mConnectionState == VeQItemMqttProducer::Connecting
				|| mConnectionState == VeQItemMqttProducer::Reconnecting)) {
		// If the initial connection failed, QMqttClient might fail
		// to emit the state change correctly.  Force it,
		// but use a queued connection to avoid the possibility
		// that the state change signal gets emitted after the
		// error change signal.
		QMetaObject::invokeMethod(this, [this] {
			onStateChanged(QMqttClient::Disconnected);
		}, Qt::QueuedConnection);
	}
}

void VeQItemMqttProducer::onStateChanged(QMqttClient::ClientState state)
{
	if (mMqttConnection && mMqttConnection->state() == QMqttClient::Disconnected
			&& (mConnectionState == VeQItemMqttProducer::Connecting
				|| mConnectionState == VeQItemMqttProducer::Reconnecting)) {
		// if the connection attempt failed, trigger our normal onDisconnected handler.
		onDisconnected();
	}
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
				mMqttConnection->unsubscribe(QStringLiteral("N/+/system/0/Serial"));
				QObject::disconnect(mMqttConnection, &QMqttClient::messageReceived,
					this, &VeQItemMqttProducer::onMessageReceived);
				mMqttSubscription = mMqttConnection->subscribe(QStringLiteral("N/%1/#").arg(mPortalId));
				QObject::connect(mMqttSubscription.data(), &QMqttSubscription::messageReceived,
					this, &VeQItemMqttProducer::onSubscriptionMessageReceived, Qt::UniqueConnection);
				doKeepAlive();
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
	if (!mReceivedMessage) {
		mReceivedMessage = true;
		setConnectionState(VeQItemMqttProducer::Initializing);
		// we will transition to Ready state after some time
		// even if we are still receiving initial messages,
		// just in case the broker forgets to send us the Ready message.
		mReadyStateFallbackTimer->start();
	}

	const QString topicName(message.topic().name());
	const QString notificationPrefix = QStringLiteral("N/%1").arg(mPortalId);
	if (topicName.startsWith(notificationPrefix)) {
		const QString keepaliveTopic = notificationPrefix + QStringLiteral("/keepalive");
		const QString heartbeatTopic = notificationPrefix + QStringLiteral("/heartbeat");
		const QString readyTopic = notificationPrefix + QStringLiteral("/full_publish_completed");
		if (topicName.compare(keepaliveTopic, Qt::CaseInsensitive) == 0) {
			// ignore keepalive topic.
		} else if (topicName.compare(readyTopic, Qt::CaseInsensitive) == 0
				&& connectionState() == VeQItemMqttProducer::Initializing) {
			setConnectionState(VeQItemMqttProducer::Ready);
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
// should have an empty message payload.
// Periodic keepAlive messages sent every 30 seconds thereafter should
// have the payload: { "keepalive-options" : ["suppress-republish"] }
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
			onDisconnected();
			return;
		}
#endif
		if (mIsVrmBroker || !suppressRepublish) {
			mMqttConnection->publish(QMqttTopicName(QStringLiteral("R/%1/keepalive").arg(mPortalId)),
					QByteArray());
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
		emit connectionStateChanged();
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
		emit errorChanged();
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
		emit portalIdChanged();
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

//--

#ifdef MQTT_WEBSOCKETS_ENABLED
WebSocketDevice::WebSocketDevice(QObject *parent)
	: QIODevice(parent)
{
	connect(&mWebSocket, &QWebSocket::stateChanged,
		this, [this] {
			if (mWebSocket.state() == QAbstractSocket::ClosingState) {
				if (isOpen()) {
					QIODevice::close();
				}
			} else if (mWebSocket.state() == QAbstractSocket::UnconnectedState) {
				Q_EMIT disconnected();
			}
		});
	connect(&mWebSocket, &QWebSocket::connected,
		this, &WebSocketDevice::connected);
	connect(&mWebSocket, &QWebSocket::disconnected,
		this, &WebSocketDevice::disconnected);
	connect(&mWebSocket, &QWebSocket::binaryMessageReceived,
		this, &WebSocketDevice::onBinaryMessageReceived);
}

void WebSocketDevice::setUrl(const QUrl &url)
{
	if (mUrl != url) {
		mUrl = url;
		emit urlChanged();
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
		emit protocolChanged();
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
	// FIXME: Qt 6.4 supports websocket subprotocols, but until then...
	QNetworkRequest r;
	r.setUrl(mUrl);
	r.setRawHeader("Sec-WebSocket-Protocol", mProtocol.constData());
	mWebSocket.open(r);
	return QIODevice::open(mode);
}

void WebSocketDevice::close()
{
	if (mWebSocket.isValid())
		mWebSocket.close();
	QIODevice::close();
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

void WebSocketDevice::onBinaryMessageReceived(const QByteArray &message)
{
	mData.append(message);
	emit readyRead();
}

qint64 WebSocketDevice::bytesAvailable() const
{
	return mData.size() + QIODevice::bytesAvailable();
}

bool WebSocketDevice::isSequential() const
{
	return true;
}
#endif // MQTT_WEBSOCKETS_ENABLED

