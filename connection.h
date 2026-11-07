#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

class Connection
{
public:
    static Connection& createInstance();
    bool createConnection();

private:
    QSqlDatabase db;
    Connection();
    ~Connection();

    // Empêcher la copie pour le pattern Singleton
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H