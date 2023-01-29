#include "database.h"

dataBase::~dataBase() {
}

dataBase::dataBase(QObject *parent)
    : QObject(parent) {
}

void dataBase::connectToDB() {
    qDebug() << "connectToDB: " << NAME_BASE;
    if (!QFile(NAME_BASE).exists()) {
        this->restoreDataBase();
    } else {
        this->openDataBase();
    }
}

bool dataBase::restoreDataBase() {
    if(this->openDataBase())
        return createTables();
    return false;
}

bool dataBase::openDataBase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(NAME_BASE);
    return db.open();
}

void dataBase::closeDataBase() {
    db.close();
}

bool dataBase::createTables() {
    QString lastSessionPositions = ( "CREATE TABLE " TABLE_POSITIONS " (           \n"
                                            "id           INTEGER PRIMARY KEY,     \n"
                                            "color_cell   INTEGER   NOT NULL,      \n"
                                            "is_busy_cell bool      NOT NULL );    \n"
                                    );
    QString lastSessionInformation = ( "CREATE TABLE " TABLE_INFO " (              \n"
                                            "id           INTEGER PRIMARY KEY,     \n"
                                            "score        INTEGER)                 \n"
                                      );
    bool res1 = querySQL(lastSessionPositions);
    bool res2 = querySQL(lastSessionInformation);
    return res1 && res2;
}

QJsonArray dataBase::getLastProgressPosition() {
    QString queryStr = (" SELECT * FROM " TABLE_POSITIONS);
    return querySQLJS(queryStr);
}

QJsonArray dataBase::getLastInformation() {
    QString queryStr = (" SELECT * FROM " TABLE_INFO);
    return querySQLJS(queryStr);
}

void dataBase::insertNewPosition(int id, const Cell cell) {
    QString queryStr = QString(" INSERT INTO " TABLE_POSITIONS
                               " (id, color_cell, is_busy_cell) "
                               " values(" + QString::number(id) +
                               ", "+ QString::number(cell.getNumColor()) +
                               ", " + QString::number(cell.isBusy) + ") ");
    querySQL(queryStr);
}

void dataBase::insertNewBasicInfo(int id) {
    QString queryStr = QString(" INSERT INTO " TABLE_INFO
                               " (id, score) "
                               " values(" + QString::number(id) + ", " +
                               QString::number(0) +" )");
    querySQL(queryStr);
}

void dataBase::updateStatusPosition(int id, const Cell cell) {
    QString queryStr = (" UPDATE " TABLE_POSITIONS
                        " SET color_cell = " + QString::number(cell.getNumColor()) + ", "
                        " is_busy_cell = " + QString::number(cell.isBusy) +
                        " WHERE id = " + QString::number(id));
    querySQL(queryStr);
}

void dataBase::updateScore(int id, int score) {
    QString queryStr = (" UPDATE " TABLE_INFO
                        " SET score = " + QString::number(score) +
                        " WHERE id = " + QString::number(id));
    querySQL(queryStr);
}

void dataBase::clearTablePositions() {
    QString queryStr = (" DELETE FROM " TABLE_POSITIONS);
    querySQL(queryStr);
}

void dataBase::clearTableInformation() {
    QString queryStr = (" DELETE FROM " TABLE_INFO);
    querySQL(queryStr);
}

bool dataBase::querySQL(const QString query) {
    if (db.isOpen()) {
        QSqlQuery querySQL(db);
        bool res;
        querySQL.setForwardOnly(true);
        res = querySQL.exec(query);

        if (!res) {
            qDebug() << "ERROR in querySQLJS: " + querySQL.lastError().text() << " \n"
                     << "WITH query: " << query;
            return res;
        }
        return res;
    }
    qDebug() << "ERROR in querySQL: DataBase in not open";
    return false;
}


QJsonArray dataBase::querySQLJS(const QString query) {
    QJsonArray retQuery;
    if (db.isOpen()) {
        QSqlQuery querySQL(db);
        bool res;
        querySQL.setForwardOnly(true);
        res = querySQL.exec(query);
        if (!res) {
            qDebug() << "ERROR in querySQLJS: " + querySQL.lastError().text() << " \n"
                     << "WITH query: " << query;
            return retQuery;
        }
        QSqlRecord fields = querySQL.record();
        while (querySQL.next()){
            QVariantMap Jline;
            for (int i = 0 ;i < fields.count(); ++i)
                if (querySQL.value(i).isNull()) {
                    Jline.insert(fields.fieldName(i), "");
                }
                else {
                    Jline.insert(fields.fieldName(i), querySQL.value(i).toString());
                }
            retQuery.append(QJsonValue(QJsonObject::fromVariantMap(Jline)));
        }
        return retQuery;
    }
    qDebug() << "ERROR in querySQLJS: DataBase in not open";
    return retQuery;
}
