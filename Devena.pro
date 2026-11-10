
QT += core gui widgets printsupport charts network sql multimedia

CONFIG += c++17
TEMPLATE = app
TARGET = Devena

SOURCES += \
    main.cpp \
    participant.cpp \
    connection.cpp

HEADERS += \
    participant.h \
    connection.h

# Lecture des CV au format PDF
greaterThan(QT_MAJOR_VERSION, 5) {
    qtHaveModule(pdf) {
        QT += pdf
        DEFINES += HAS_QTPDF
    }
}
