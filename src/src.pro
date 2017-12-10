include(../defaults.pri)
DESTDIR = $$BINDIR

TARGET = JSONToRPCBridge

TEMPLATE = lib

DEFINES += EXPORT_LIBRARY


QPROTOCOL_INTERPRETER_PATH=$$PWD/../libs/qRPCRuntimeParser
INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/src
include($$QPROTOCOL_INTERPRETER_PATH/qProtocollInterpreter_static.pri)




LIBS += -L$$BINDIR

#QMAKE_CXXFLAGS += --time-report
#QMAKE_CXXFLAGS += -flto
#QMAKE_LFLAGS += -fno-use-linker-plugin -flto

HEADERS +=    jsoninput.h
HEADERS +=    rpcserialport.h
HEADERS +=    mainclass.h
HEADERS +=    rpcprotocol.h
#HEADERS +=    serialworker.h
HEADERS +=    qt_util.h
HEADERS +=    vc.h

SOURCES +=     jsoninput.cpp
SOURCES +=     rpcserialport.cpp
SOURCES +=     rpcprotocol.cpp
SOURCES +=     mainclass.cpp
#SOURCES +=     serialworker.cpp
SOURCES +=     qt_util.cpp

win32 {
    SH = C:/Program Files/Git/bin/sh.exe
}else{
    SH = sh
}



system($$system_quote($$SH) $$PWD/../git.sh)
