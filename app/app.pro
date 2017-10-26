include(../defaults.pri)
DESTDIR = $$BINDIR

TEMPLATE = app
QT       -= gui


TARGET = consoleapp
CONFIG   += console
CONFIG   -= app_bundle

QPROTOCOL_INTERPRETER_PATH=$$PWD/../libs/qRPCRuntimeParser

INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/src
INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/libs/RPC-ChannelCodec/include
INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/libs/include/
INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/libs/RPC-ChannelCodec/include/errorlogger_dummy
INCLUDEPATH += $$QPROTOCOL_INTERPRETER_PATH/project/libs/RPC-ChannelCodec/tests/include

SOURCES += main.cpp
DEFINES += EXPORT_APPLICATION

LIBS += -L$$BINDIR
LIBS +=  -lJSONToRPCBridge

