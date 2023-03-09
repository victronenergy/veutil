#include <veutil/qt/ve_qitems_mqtt.hpp>

#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>

//--

VeQItemMqtt::VeQItemMqtt(VeQItemMqttProducer *producer)
	: VeQItem(producer, nullptr)
{
}

int VeQItemMqtt::setValue(QVariant const &value)
{
	if (!mqttProducer() || !mqttProducer()->publishValue(uniqueId(), value)) {
		return -1;
	}
	return 0;
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
	  mMqttConnection(nullptr),
	  mPort(0),
	  mConnectionState(Idle),
	  mAutoReconnectAttemptCounter(0),
	  mAutoReconnectMaxAttempts(sizeof(mReconnectAttemptIntervals)/sizeof(mReconnectAttemptIntervals[0])),
	  mError(QMqttClient::NoError),
	  mProtocolVersion(QMqttClient::MQTT_3_1_1),
	  mReceivedMessage(false)
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

	mKeepAliveTimer.setInterval(1000 * 30);
	connect(&mKeepAliveTimer, &QTimer::timeout,
		this, &VeQItemMqttProducer::doKeepAlive);
	mKeepAliveTimer.start();
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
		mAutoReconnectAttemptCounter = 0;

		connect(mMqttConnection, &QMqttClient::connected,
			this, &VeQItemMqttProducer::onConnected);
		connect(mMqttConnection, &QMqttClient::disconnected,
			this, &VeQItemMqttProducer::onDisconnected);
		connect(mMqttConnection, &QMqttClient::errorChanged,
			this, &VeQItemMqttProducer::onErrorChanged);
		connect(mMqttConnection, &QMqttClient::messageReceived,
			this, &VeQItemMqttProducer::onMessageReceived);

		if (mWebSocket) {
			mWebSocket->deleteLater();
		}

		mWebSocket = new WebSocketDevice(mMqttConnection);
		mWebSocket->setUrl(url);
		mWebSocket->setProtocol(
			  protocolVersion == QMqttClient::MQTT_3_1 ? "mqttv3.1" : "mqtt");

		connect(mWebSocket, &WebSocketDevice::connected,
			this, [this, protocolVersion] {
				mMqttConnection->setProtocolVersion(protocolVersion);
				mMqttConnection->setTransport(mWebSocket, QMqttClient::IODevice);
				QMetaObject::invokeMethod(this, [this] { aboutToConnect(); }, Qt::QueuedConnection);
			});
		connect(mWebSocket, &WebSocketDevice::disconnected,
			this, [this] {
				// TODO: does QMqttClient handle this already?
				// Or do I need to manually close()?
				qWarning() << "WebSocket disconnected!";
				mWebSocket->close();
				mMqttConnection->disconnected();
			});

		setConnectionState(Connecting);
		mWebSocket->open(QIODeviceBase::ReadWrite);
	}, Qt::QueuedConnection);
}
#endif // MQTT_WEBSOCKETS_ENABLED

void VeQItemMqttProducer::open(const QHostAddress &host, int port)
{
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
		connect(mMqttConnection, &QMqttClient::messageReceived,
			this, &VeQItemMqttProducer::onMessageReceived);

		mAutoReconnectAttemptCounter = 0;
		QMetaObject::invokeMethod(this, [this] { aboutToConnect(); }, Qt::QueuedConnection);
		setConnectionState(Connecting);
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
		mMqttConnection->setCleanSession(true);
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
		mMqttConnection->subscribe(QStringLiteral("N/%1/#").arg(mPortalId));
		mMqttConnection->publish(QMqttTopicName(QStringLiteral("R/%1/system/0/Serial").arg(mPortalId)), QByteArray());
		doKeepAlive();
	}

	mKeepAliveTimer.start();
}

void VeQItemMqttProducer::onDisconnected()
{
	setConnectionState(Disconnected);
	mKeepAliveTimer.stop();
	mMessageQueue.clear();
	mReceivedMessage = false;

	if (mAutoReconnectAttemptCounter < mAutoReconnectMaxAttempts) {
		// Attempt to reconnect.  We use a staggered exponential backoff interval.
		setConnectionState(Reconnecting);
		const int interval = mReconnectAttemptIntervals[mAutoReconnectAttemptCounter++];
#ifdef MQTT_WEBSOCKETS_ENABLED
		if (!mWebSocket || !mWebSocket->isValid()) {
			QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
					this, [this] {
						quint16 count = mAutoReconnectAttemptCounter;
						if (mHostName.isEmpty()) {
							open(mUrl, mProtocolVersion);
						} else {
							open(QHostAddress(mHostName), mPort);
						}
						mAutoReconnectAttemptCounter= count;
					});
		} else {
			QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
					this, &VeQItemMqttProducer::aboutToConnect);
		}
#else
		QTimer::singleShot(interval + QRandomGenerator::global()->bounded(interval/2),
				this, &VeQItemMqttProducer::aboutToConnect);
#endif
	} else {
		setConnectionState(Failed);
	}
}

void VeQItemMqttProducer::onErrorChanged(QMqttClient::ClientError error)
{
	setError(error);
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
				mMqttConnection->subscribe(QStringLiteral("N/%1/#").arg(mPortalId));
				doKeepAlive();
			} else {
				qWarning() << "VeQItemMqttProducer::onMessageReceived(): portal id mismatch: "
					<< topicName << " -> " << QString::fromUtf8(message);
			}
		}
	} else {
		const QString notificationPrefix = QStringLiteral("N/%1").arg(mPortalId);
		if (topicName.startsWith(notificationPrefix)) {
			const QString keepaliveTopic = notificationPrefix + QStringLiteral("/keepalive");
			if (topicName.compare(keepaliveTopic, Qt::CaseInsensitive) == 0) {
				// ignore keepalive topic.
			} else {
				// we have a topic message which we need to expose via VeQItem.
				// service the queue via a QueuedConnection to ensure
				// that we don't block the UI.
				const QString path = topicName.mid(notificationPrefix.size() + 1);
				mMessageQueue.enqueue(QPair<QString, QByteArray>(path, message));
				if (mMessageQueue.size() == 1) {
					QMetaObject::invokeMethod(this, [this] {
						serviceQueue();
					}, Qt::QueuedConnection);
				}
			}
		}

		// Once we have received a message, transition to Ready state.
		if (!mReceivedMessage) {
			mReceivedMessage = true;
			doKeepAlive();
		}
	}
}

void VeQItemMqttProducer::serviceQueue()
{
	if (mMessageQueue.size()) {
		const QPair<QString, QByteArray> message = mMessageQueue.dequeue();
		parseMessage(message.first, message.second);
		QMetaObject::invokeMethod(this, [this] {
			serviceQueue();
		}, Qt::QueuedConnection);
	}
}

void VeQItemMqttProducer::parseMessage(const QString &path, const QByteArray &message)
{
	VeQItemMqtt *item = qobject_cast<VeQItemMqtt*>(mProducerRoot->itemGetOrCreate(path, true, true));

	// if the payload is null/empty, then the device has been removed from the system.
	// for now, simply ignore the message.  In future, we might (after some timeout)
	// do item->itemDelete() or similar.
	if (message.size() == 0) {
		Q_EMIT nullMessageReceived(path);
	} else {
		// otherwise, update the value in the item.
		// TODO: text / min / max / default ??
		const QJsonObject payload = QJsonDocument::fromJson(message).object();
		const QVariant variant = payload.value(QStringLiteral("value")).toVariant();
		item->produceValue(variant.isNull() ? QVariant() : variant); // work around QJsonValue always using std::nullptr_t even for literal null values.
		Q_EMIT messageReceived(path, variant);
	}
}

void VeQItemMqttProducer::doKeepAlive()
{
	if (mMqttConnection
			&& mMqttConnection->state() == QMqttClient::Connected
			&& !mPortalId.isEmpty()) {
		if (mReceivedMessage) {
			setConnectionState(Ready);
		}
		mMqttConnection->publish(QMqttTopicName(QStringLiteral("R/%1/keepalive").arg(mPortalId)), QByteArray());
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

//--

#ifdef MQTT_WEBSOCKETS_ENABLED
WebSocketDevice::WebSocketDevice(QObject *parent)
	: QIODevice(parent)
{
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
#endif // MQTT_WEBSOCKETS_ENABLED

