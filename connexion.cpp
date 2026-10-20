#include "connexion.h"
#include <QSqlError>
#include <QDebug>

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen()) {
        db.close();
    }
}

Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    db.setDatabaseName("devena");
    db.setUserName("devena");
    db.setPassword("2006");

    if (db.open()) {
        qDebug() << "Connexion Oracle établie avec succès !";
        return true;
    }

    qDebug() << "Échec de la connexion Oracle :"
             << db.lastError().text();

    return false;
}

