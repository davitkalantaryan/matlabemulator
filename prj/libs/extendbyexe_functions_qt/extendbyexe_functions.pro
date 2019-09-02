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
include ($${PWD}/../../../contrib/matlab/prj/common/common_qt/matlab_matrix_without_libs_common.pri)

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../src/include_p

QT -= gui
QT -= core
#greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

#CONFIG -= RPATH
#QMAKE_RPATHDIR = /afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/bin/glnxa64
#QMAKE_RPATHDIR = $${PWD}/../../../sys/Nitrgen/dll

DEFINES += MATEMUL_EE_CREATING_DYN_LIB

win32{
    SOURCES += \

} else {
    #LIBS += -lpthread
    SOURCES += \

}

SOURCES += \
    $${PWD}/../../../src/libs/dllmain_libextendbyexe_functions.cpp

HEADERS += \
    $${PWD}/../../../include/common/matlabemulator_compiler_internal.h          \
    $${PWD}/../../../include/matlab/emulator/extendbylib_common.hpp             \
    $${PWD}/../../../include/matlab/emulator/extendbylib_print_functions.hpp    \
    $${PWD}/../../../include/matlab/emulator/extendbylib_other_functions.hpp    \
    $${PWD}/../../../include/matlab/emulator/extendbyexe_functions.hpp          \
    $${PWD}/../../../src/include_p/extendbylib_functions_private.hpp

OTHER_FILES += \
