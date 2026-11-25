#include "connection.h"
#include "participant.h"
#include <QMessageBox>
#include <QApplication>
#include <QSqlQuery>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Récupération de l'instance unique du singleton de connexion
    Connection& c = Connection::createInstance();

    // Tentative d'établissement de la connexion avec Oracle
    bool test = c.createConnection();

    MainWindow W;

    if (test) {
        QMessageBox::information(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Connexion réussie à la base de données Oracle.\nBienvenue dans Devena."),
            QMessageBox::Ok
            );
        W.show();
    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion à la base de données.\nVeuillez vérifier vos paramètres de configuration Oracle."),
            QMessageBox::Cancel
            );
        return 1;
    }

    return a.exec();
}
