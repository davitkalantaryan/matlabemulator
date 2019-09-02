#
# file:         emulator.pro
# created on:   2019 Jun 06
#

#TEMPLATE = app
TEMPLATE = lib
#TARGET = matlabemulator

CONFIG += DEBUG

#CONFIG += c++17
#QMAKE_CXXFLAGS += -std=c++14

GCCPATH = $$system(which gcc)

message("!!!!!!!!!!! GCCPATH=$$GCCPATH")

#include ($${PWD}/../../common/common_qt/sys_common.pri)

INCLUDEPATH += $${PWD}/../../../include

QT -= gui
QT -= core
#greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

#CONFIG -= RPATH
#QMAKE_RPATHDIR = /afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/bin/glnxa64
#QMAKE_RPATHDIR = $${PWD}/../../../sys/Nitrgen/dll

win32{
    SOURCES += \

} else {
    #LIBS += -lpthread
    SOURCES += \

}

SOURCES += \
    $${PWD}/../../../src/tests/dllmain_libextend2_test.cpp

HEADERS += \

OTHER_FILES += \
