#include "connexion.h"
#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // On récupère l’instance unique du singleton
    Connection& c = Connection::createInstance();

    // On tente d’établir la connexion
    bool test = c.createConnection();

    //************************Test Connection BD**************************//
    //*******************************************************************//
    if (!test) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion.\nCliquez sur Annuler pour quitter."),
            QMessageBox::Cancel
            );
        return 1;
    }

    // MainWindow lit les projets dès son constructeur : il doit être créé après la connexion.
    MainWindow W;
    W.show();
    QMessageBox::information(
        nullptr,
        QObject::tr("Base de données"),
        QObject::tr("Connexion réussie.\nCliquez sur Annuler pour quitter."),
        QMessageBox::Cancel
        );

    return a.exec();
}
