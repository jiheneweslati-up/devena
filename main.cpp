#include "connection.h"
#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection c;
    if (!c.ouvrir()) {
        QMessageBox::critical(nullptr, QObject::tr("Base de données"),
                              QObject::tr("Connexion à Oracle impossible.\n\n%1")
                                  .arg(c.derniereErreur()));
        return 1;
    }

    MainWindow w;
    w.show();
    const int code = QApplication::exec();
    c.fermer();
    return code;
}
