#
# file:         emulator.pro
# created on:   2019 Jun 06
#

TEMPLATE = app
TARGET = matlabemulator

# include ($${PWD}/../../common/common_qt/sys_common.pri)


QT += gui
QT += core
greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

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
