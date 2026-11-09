
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Connexion Singleton
    Connection& C = Connection::createInstance();
    bool test = C.createConnection();

    if (test)
    {
        QMessageBox::information(
            nullptr,
            "Connexion Oracle",
            "Connexion réussie !"
            );

        // Création de la fenêtre après connexion
        MainWindow w;
        w.show();

        return a.exec();
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
