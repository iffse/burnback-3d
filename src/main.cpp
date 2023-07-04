#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>

#include <src/headers/globals.h>
#include <src/headers/interface.h>

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication app(argc, argv);

	QQmlApplicationEngine engine;

	// Set the context property to access the class from QML
	QQmlContext *rootContext = engine.rootContext();
	Actions actions;
	rootContext->setContextProperty("actions", &actions);

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(
	    &engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
		    if (!obj && url == objUrl)
			    QCoreApplication::exit(-1);
	    },
	    Qt::QueuedConnection);
	engine.load(url);

	// Get the root object in order to access children properties
	root = engine.rootObjects().at(0);

	// Use monospaced font for output
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	QObject *output = root->findChild<QObject *>("output");
	output->setProperty("font", fixedFont);

	// create temporary directory path to store isosurfaces
	tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if (tmpDir == "") {
		tmpDir = QDir::currentPath();
	}
	if (!tmpDir.endsWith("/")) {
		tmpDir += "/";
	}
	tmpDir += "burnback-3d/";

	return app.exec();
}
