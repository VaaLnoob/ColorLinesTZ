#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "database.h"
#include "gameboard.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(5,12,0)
    QCoreApplication::setAttribure(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    dataBase DB;
    gameBoard* pBoard = new gameBoard();

    DB.connectToDB();
    pBoard->appDb = &DB;
    pBoard->refresh();

    engine.rootContext()->setContextProperty("BoardLink", pBoard);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *object, const QUrl &objectUrl){
        if(!object && url == objectUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
