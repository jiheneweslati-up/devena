#include "connection.h"

#include <QSqlDatabase>
#include <QSqlError>

// A ADAPTER : nom de la source de donnees ODBC (DSN) creee dans
// "Sources de donnees ODBC (64 bits)" de Windows, et ton compte Oracle.
static const char *NOM_DSN      = "devena";
static const char *UTILISATEUR  = "devena";
static const char *MOT_DE_PASSE = "2006";

bool Connection::ouvrir()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(NOM_DSN);
    db.setUserName(UTILISATEUR);
    db.setPassword(MOT_DE_PASSE);

    if (!db.open()) {
        m_erreur = db.lastError().text();
        return false;
    }
    return true;
}

void Connection::fermer()
{
    QSqlDatabase::database().close();
}
