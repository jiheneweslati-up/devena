QT += core gui widgets sql printsupport

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    projet.cpp \
    employe.cpp \
    connection.cpp \
    SessionManager.cpp

HEADERS += \
    mainwindow.h \
    projet.h \
    employe.h \
    connection.h \
    SessionManager.h

FORMS += \
    employe.ui

RESOURCES += \
    resources.qrc
