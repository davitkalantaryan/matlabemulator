#
# file:         emulator.pro
# created on:   2019 Jun 06
#

TEMPLATE = app
TARGET = matlabemulator

CONFIG += DEBUG

#CONFIG += c++17
#QMAKE_CXXFLAGS += -std=c++14

GCCPATH = $$system(which gcc)

message("!!!!!!!!!!! GCCPATH=$$GCCPATH")

# include ($${PWD}/../../common/common_qt/sys_common.pri)
include ($${PWD}/../../../contrib/matlab/prj/common/common_qt/matlab_matrix_common.pri)

QT += gui
QT += core
greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

LIBS += -leng
LIBS += -lmat
LIBS += -lhdf5_hl
LIBS += -lhdf5

win32{
} else {
    LIBS += -lpthread
}

SOURCES += \
    $${PWD}/../../../src/emulator/main_matlabemulator.cpp               \
    $${PWD}/../../../src/emulator/matlab_emulator_application.cpp       \
    $${PWD}/../../../src/emulator/matlab_emulator_mainwindow.cpp        \
    $${PWD}/../../../src/emulator/matlab_emulator_centralwidget.cpp     \
    $${PWD}/../../../src/emulator/matlab_emulator_commandprompt.cpp

HEADERS += \
    $${PWD}/../../../src/emulator/matlab_emulator_application.hpp       \
    $${PWD}/../../../src/emulator/matlab_emulator_mainwindow.hpp        \
    $${PWD}/../../../src/emulator/matlab_emulator_centralwidget.hpp     \
    $${PWD}/../../../src/emulator/matlab_emulator_commandprompt.hpp
