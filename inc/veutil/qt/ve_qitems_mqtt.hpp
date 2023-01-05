#pragma once

#include <veutil/qt/ve_qitem.hpp>

#include <QMqttClient>

#include <QHostAddress>
#include <QTimer>
#include <QSet>

#ifdef MQTT_WEBSOCKETS_ENABLED
#include <QUrl>
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

	VeQItemMqttProducer(VeQItem *root, const QString &id, QObject *parent = nullptr);

	VeQItem *createItem() override;

	virtual bool open(const QHostAddress &host, int port);

#ifdef MQTT_WEBSOCKETS_ENABLED
	virtual bool open(
		const QUrl &url,
		QMqttClient::ProtocolVersion protocolVersion);
#endif // MQTT_WEBSOCKETS_ENABLED

	QMqttClient *mqttConnection() const;
	bool publishValue(const QString &uid, const QVariant &value);
	ConnectionState connectionState() const;
	QMqttClient::ClientError error() const;

Q_SIGNALS:
	void connectionStateChanged();
	void errorChanged();
	void messageReceived(const QString &path, const QVariant &value);
	void nullMessageReceived(const QString &path);

private Q_SLOTS:
	void onConnected();
	void onDisconnected();
	void onErrorChanged(QMqttClient::ClientError error);
	void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
	void doKeepAlive();

private:
	void continueOpen();
	void setConnectionState(ConnectionState connectionState);
	void setError(QMqttClient::ClientError error);
	void parseMessage(const QString &path, const QByteArray &message);

	QTimer mKeepAliveTimer;
	QMqttClient *mMqttConnection;
#ifdef MQTT_WEBSOCKETS_ENABLED
	WebSocketDevice *mWebSocket = nullptr;
#endif // MQTT_WEBSOCKETS_ENABLED
	QString mPortalId;
	ConnectionState mConnectionState;
	quint16 mAutoReconnectAttemptCounter;
	const quint16 mAutoReconnectMaxAttempts;
	QMqttClient::ClientError mError;
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

