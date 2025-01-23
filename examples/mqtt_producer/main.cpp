#include <QGuiApplication>
#include <QScopedPointer>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QtDebug>

#ifdef Q_OS_WASM
#include <emscripten/val.h>
#include <emscripten.h>
#include <QWebSocketProtocol>
#include <QUrl>
#include <QUrlQuery>
#else
#include <QHostAddress>
#endif

#include <veutil/qt/ve_qitems_mqtt.hpp>
#include <veutil/qt/ve_quick_item.hpp>

int main(int argc, char *argv[])
{
	qmlRegisterType<VeQuickItem>("Victron.VeUtil", 1, 0, "VeQuickItem");

	QGuiApplication app(argc, argv);
	QQmlEngine engine;
	QQmlComponent component(&engine, QUrl(QStringLiteral("qrc:/main.qml")));

	bool running = false;
	VeQItemMqttProducer producer(VeQItems::getRoot(), QStringLiteral("mqtt"), QStringLiteral("example"));
	QObject::connect(&producer, &VeQItemMqttProducer::aboutToConnect,
		[&producer] {
			producer.setCredentials(QString(), QString());
			producer.continueConnect();
		});
	QObject::connect(&producer, &VeQItemMqttProducer::connectionStateChanged,
		[&running, &producer] {
			if (producer.connectionState() == VeQItemMqttProducer::Ready) {
				running = true;
			}
		});

	QTimer::singleShot(10000,
		[&app, &running] {
			if (!running) {
				qWarning() << "MQTT producer never became ready, unable to connect?";
				app.quit();
			}
		});

	QQuickWindow *window = qobject_cast<QQuickWindow*>(component.create());
	if (!window) {
		qWarning() << "Unable to create main.qml component! " << component.errors();
		return 1;
	}

	window->showFullScreen();
#ifdef Q_OS_WASM
	emscripten::val webLocation = emscripten::val::global("location");
	const QUrl webLocationUrl = QUrl(QString::fromStdString(webLocation["href"].as<std::string>()));
	const QUrlQuery query(webLocationUrl);
	const QString ip(query.queryItemValue("mqtt"));
	if (ip.isEmpty()) {
		qWarning() << "Please specify the broker's IP address via mqtt query parameter, e.g. ?mqtt=192.168.5.96";
	} else {
		const QString mqttUrl(QStringLiteral("ws://%1:9001/").arg(ip)); // e.g.: "ws://192.168.5.96:9001/"
		producer.open(QUrl(mqttUrl), QMqttClient::MQTT_3_1);
	}
#else
	producer.open(QHostAddress::LocalHost, 1883);
#endif
	return app.exec();
}
