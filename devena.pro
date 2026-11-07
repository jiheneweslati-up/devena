QT += widgets printsupport sql
CONFIG += c++17

SOURCES += \
   equipe.cpp \
   main.cpp \
   mainwindow.cpp \
   connection.cpp
HEADERS += \
    connection.h \
    equipe.h \
    mainwindow.h
    connection.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target