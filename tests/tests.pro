include(../defaults.pri)
DESTDIR = $$BINDIR

TEMPLATE = app
CONFIG += console
QT += testlib

DEFINES += EXPORT_APPLICATION



HEADERS +=autotest.h
HEADERS +=testgooglemock.h
HEADERS +=testqstring.h
HEADERS +=jsoninputtest.h


SOURCES += main.cpp
SOURCES += testgooglemock.cpp
SOURCES += testqstring.cpp
SOURCES += jsoninputtest.cpp

INCLUDEPATH += $$PWD/../libs/googletest/googletest/include
INCLUDEPATH += $$PWD/../libs/googletest/googlemock/include

LIBS += -L$$BINDIR
LIBS += -L$$PWD/../libs/googletest/build
LIBS += -lgmock
LIBS += -lgtest
LIBS += -lJSONToRPCBridge

COPY_DIR = "$$(UNIXTOOLS)cp -r"

#copies scripts into builds

#runtests.commands = $$RUNTEST
#runtests.depends = copydata

copydata.commands = $$COPY_DIR $$PWD/scripts $$OUT_PWD/
first.depends = $(first) copydata

export(first.depends)
export(copydata.commands)

QMAKE_EXTRA_TARGETS += first copydata
