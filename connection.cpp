#include "connection.h"

// Constructeur
Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

// Destructeur
Connection::~Connection()
{
    if(db.isOpen())

    db.close();
}

// Singleton Pattern
Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

// Fonction de Connexion
bool Connection::createConnection()
{
    qDebug() << "Drivers ODBC disponibles :" << QSqlDatabase::drivers();

    db.setDatabaseName("Source_ProjetCpp");
    db.setUserName("devena");
    db.setPassword("esprit2026");

    if(db.open()){
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Echec de la connexion :" << db.lastError().text();
    return false;
}