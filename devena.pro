QT += core gui widgets sql printsupport charts network multimedia
CONFIG += c++17
TEMPLATE = app
TARGET = devena

# Application integree Devena :
#   AppShell (login + tableau de bord) et module Employes : Rayan
#   module Participants : Nour     -   module Equipes : Alaa
#   module Projets : Jihene        -   module Evaluations : Amin
SOURCES += \
    main.cpp \
    appshell.cpp \
    connection.cpp \
    SessionManager.cpp \
    employee.cpp \
    participant.cpp \
    equipe.cpp \
    equipepage.cpp \
    projet.cpp \
    mainwindow.cpp \
    evaluation.cpp \
    evaluationwindow.cpp

HEADERS += \
    appshell.h \
    connection.h \
    SessionManager.h \
    employee.h \
    participant.h \
    equipe.h \
    equipepage.h \
    projet.h \
    mainwindow.h \
    evaluation.h \
    evaluationwindow.h

FORMS += \
    mainwindow.ui \
    evaluationwindow.ui

# Module Participants : lecture des CV au format PDF (si le module Qt PDF est installe)
greaterThan(QT_MAJOR_VERSION, 5) {
    qtHaveModule(pdf) {
        QT += pdf
        DEFINES += HAS_QTPDF
    }
}
