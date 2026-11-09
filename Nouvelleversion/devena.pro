
QT += widgets printsupport sql

CONFIG += c++17

SOURCES += \
    Connection.cpp \
    equipe.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Connection.h \
    equipe.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
