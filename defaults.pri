CONFIG += c++14
#QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -std=c++14
QT =  core serialport xml



INCLUDEPATH += $$PWD/src

INCLUDEPATH += $$PWD/libs/qRPCRuntimeParser/project/libs/include/



win32 {




}else{

}




QMAKE_CXXFLAGS += -Werror

QMAKE_CXXFLAGS_DEBUG += -Wall -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare
QMAKE_CXXFLAGS_RELEASE += -Wall -Wunused-function -Wunused-parameter -Wunused-variable

QMAKE_CXXFLAGS_DEBUG += -g -fno-omit-frame-pointer
#QMAKE_CXXFLAGS_DEBUG += -fsanitize=undefined,address
#QMAKE_CXXFLAGS_DEBUG += -static-libasan -static-libubsan #some day windows will support a reasonable development environment ...

CONFIG(debug, debug|release) {
    BINSUB_DIR = bin/debug
    BINDIR = $$OUT_PWD/../$$BINSUB_DIR#

} else {
    BINSUB_DIR = bin/release
    BINDIR = $$OUT_PWD/../$$BINSUB_DIR

}
