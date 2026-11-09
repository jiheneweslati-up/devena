QT += widgets printsupport sql
CONFIG += c++17

SOURCES += \
   main.cpp \
   mainwindow.cpp \
   connection.cpp \
   projet.cpp
HEADERS += \
    connection.h \
    mainwindow.h \
    projet.h
    connection.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target