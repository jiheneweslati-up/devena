#include "connection.h"
#include "participant.h"
#include <QMessageBox>
#include <QApplication>
#include <QSqlQuery>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. Récupération de l'instance unique du singleton de connexion
    Connection& c = Connection::createInstance();

    // 2. Tentative d'établissement de la connexion avec la base de données Oracle
    bool test = c.createConnection();

    MainWindow W;

    // 3. Vérification de l'état de la connexion
    if (test) {
        QMessageBox::information(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Connexion réussie à la base de données Oracle.\nBienvenue dans Devena."),
            QMessageBox::Ok
            );
        // Affichage de la fenêtre principale après succès de la connexion
        W.show();
    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion à la base de données.\nVeuillez vérifier vos paramètres de configuration Oracle."),
            QMessageBox::Cancel
            );
        return 1; // Arrêt de l'application si la connexion échoue
    }

    return a.exec();
}