#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql/QSql>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QFile>
#include <QtSql/QSqlRecord>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>

#include "struct.h"

#define VERSION_BASE    "1"
#define NAME_BASE       "ColorLinesDB" VERSION_BASE ".db"

#define TABLE_INFO      "LastSessionInformation"
#define TABLE_POSITIONS "LastSessionPositions"

class dataBase : public QObject
{
    Q_OBJECT
public:
    dataBase(QObject *parent = 0);
    ~dataBase();

    void connectToDB();

    QJsonArray getLastProgressPosition();
    QJsonArray getLastInformation();

    void insertNewPosition(int id, const Cell cell);
    void insertNewBasicInfo(int id);
    void updateStatusPosition(int id, const Cell cell);
    void updateScore(int id, int score);
    void clearTablePositions();
    void clearTableInformation();
private:
    QSqlDatabase db;
    //Делает запрос к базе
    bool       querySQL(const QString query);
    //Делает запрос с возвращаемым значением
    QJsonArray querySQLJS(const QString query);

    bool openDataBase();
    bool restoreDataBase();
    void closeDataBase();
    bool createTables();
};

#endif // DATABASE_H
