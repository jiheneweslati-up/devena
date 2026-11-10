QT       += core gui widgets sql charts printsupport

CONFIG   += c++17
TARGET    = Devena
TEMPLATE  = app

SOURCES += \
    main.cpp \
    connexion.cpp \
    evaluation.cpp

HEADERS += \
    header.h \
    connexion.h \
    evaluation.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    database/devena.sql \
    README.md
