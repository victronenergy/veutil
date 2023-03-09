#pragma once

#include <veutil/qt/ve_qitem.hpp>

#include <QMqttClient>

#include <QHostAddress>
#include <QTimer>
#include <QSet>
#include <QQueue>
#include <QString>
#include <QByteArray>
#include <QUrl>

#ifdef MQTT_WEBSOCKETS_ENABLED
#include <QWebSocket>
#include <QWebSocketProtocol>
#include <QIODevice>
class WebSocketDevice;
#endif // MQTT_WEBSOCKETS_ENABLED

class VeQItemMqttProducer;

// Implementation of a VeQItem over MQTT
class VeQItemMqtt : public VeQItem
{
	Q_OBJECT

public:
	VeQItemMqtt(VeQItemMqttProducer *producer);

	int setValue(QVariant const &value) override;

protected:
	void setParent(QObject *parent) override;

private:
	VeQItemMqttProducer *mqttProducer() const;
};

class VeQItemMqttProducer : public VeQItemProducer
{
	Q_OBJECT
	Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
	Q_PROPERTY(QMqttClient::ClientError error READ error NOTIFY errorChanged)
	Q_PROPERTY(QString portalId READ portalId WRITE setPortalId NOTIFY portalIdChanged)

public:

	enum ConnectionState {
		Idle,
		Connecting,
		Connected,
		Ready,
		Disconnected,
		Reconnecting,
		Failed
	};
	Q_ENUM(ConnectionState)

	VeQItemMqttProducer(VeQItem *root, const QString &id, const QString &clientIdPrefix, QObject *parent = nullptr);

	VeQItem *createItem() override;

	void open(const QHostAddress &host, int port);

#ifdef MQTT_WEBSOCKETS_ENABLED
	void open(
		const QUrl &url,
		QMqttClient::ProtocolVersion protocolVersion);
#endif // MQTT_WEBSOCKETS_ENABLED

	QMqttClient *mqttConnection() const;
	void setCredentials(const QString &username, const QString &password);
	bool publishValue(const QString &uid, const QVariant &value);
	ConnectionState connectionState() const;
	QMqttClient::ClientError error() const;

	QString portalId() const;
	void setPortalId(const QString &portalId);

Q_SIGNALS:
	void connectionStateChanged();
	void errorChanged();
	void portalIdChanged();
	void messageReceived(const QString &path, const QVariant &value);
	void nullMessageReceived(const QString &path);
	void aboutToConnect(); // client should handle this by calling setCredentials() and calling continueConnect

public Q_SLOTS:
	void continueConnect();

private Q_SLOTS:
	void onConnected();
	void onDisconnected();
	void onErrorChanged(QMqttClient::ClientError error);
	void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
	void doKeepAlive();

private:
	void setConnectionState(ConnectionState connectionState);
	void setError(QMqttClient::ClientError error);
	void parseMessage(const QString &path, const QByteArray &message);
	void serviceQueue();

	QTimer mKeepAliveTimer;
	QMqttClient *mMqttConnection;
#ifdef MQTT_WEBSOCKETS_ENABLED
	WebSocketDevice *mWebSocket = nullptr;
#endif // MQTT_WEBSOCKETS_ENABLED
	QString mClientId;
	QString mPortalId;
	QUrl mUrl;
	QString mHostName;
	int mPort;
	QQueue<QPair<QString, QByteArray> > mMessageQueue;
	ConnectionState mConnectionState;
	const int mReconnectAttemptIntervals[6] = { 250, 1000, 2000, 5000, 10000, 30000 };
	quint16 mAutoReconnectAttemptCounter;
	const quint16 mAutoReconnectMaxAttempts;
	QMqttClient::ClientError mError;
	QMqttClient::ProtocolVersion mProtocolVersion;
	bool mReceivedMessage;
};

#ifdef MQTT_WEBSOCKETS_ENABLED
class WebSocketDevice : public QIODevice
{
	Q_OBJECT
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QByteArray protocol READ protocol WRITE setProtocol NOTIFY protocolChanged)

public:
	WebSocketDevice(QObject *parent = nullptr);

	void setUrl(const QUrl &url);
	QUrl url() const;

	void setProtocol(const QByteArray &protocol);
	QByteArray protocol() const;

	bool isValid() const;
	bool open(QIODeviceBase::OpenMode mode) override;
	void close() override;
	qint64 readData(char *data, qint64 maxSize) override;
	qint64 writeData(const char *data, qint64 maxSize) override;

Q_SIGNALS:
	void urlChanged();
	void protocolChanged();
	void connected();
	void disconnected();

public Q_SLOTS:
	void onBinaryMessageReceived(const QByteArray &message);

private:
	QUrl mUrl;
	QByteArray mProtocol;
	QByteArray mData;
	QWebSocket mWebSocket;
};
#endif // MQTT_WEBSOCKETS_ENABLED

