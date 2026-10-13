#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AtmBridge.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    AtmBridge bridge;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("atm", &bridge);
    engine.loadFromModule("AtmApp", "Main");

    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}