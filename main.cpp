#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include "AtmBridge.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    AtmBridge bridge;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("atm", &bridge);
    engine.load(QUrl(QStringLiteral("qrc:/AtmApp/Main.qml")));
    
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}