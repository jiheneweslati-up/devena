
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialisation de la connexion Singleton
    Connection& C = Connection::createInstance();
    bool test = C.createConnection();

    if (test)
    {
        QMessageBox::information(
            nullptr,
            "Connexion Oracle",
            "Connexion réussie !"
            );
    }

    MainWindow w;

    if(test)
    {
        w.show();        // Affiche la fenêtre principale
        return a.exec(); // Lance la boucle d'événements Qt
    }
    else
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Echec de la connexion."),
            QMessageBox::Cancel
            );
        return -1;
    }
}
