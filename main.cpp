#include "appshell.h"
#include <QApplication>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Devena");
    AppShell window;
    window.show();
    return app.exec();
}
