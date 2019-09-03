#
# file:         emulator.pro
# created on:   2019 Jun 06
#

TEMPLATE = app
TARGET = matlabemulator

CONFIG += DEBUG

#QMAKE_CXXFLAGS_WARN_ON += -Wno-format-nonliteral

#CONFIG += c++17
#QMAKE_CXXFLAGS += -std=c++14


# include ($${PWD}/../../common/common_qt/sys_common.pri)
include ($${PWD}/../../../contrib/matlab/prj/common/common_qt/matlab_matrix_common.pri)

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../src/include_p

QT += gui
QT += core
greaterThan(QT_MAJOR_VERSION, 4):QT += widgets

#CONFIG -= RPATH
#QMAKE_RPATHDIR = /afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/bin/glnxa64
#QMAKE_RPATHDIR = $${PWD}/../../../sys/Nitrgen/dll

win32{
    SOURCES += \
        $${PWD}/../../../src/utils/common_system_windows.cpp
    LIBS += -llibeng
    LIBS += -llibmat
} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    include($${PWD}/../../common/common_qt/root_no_gui_common.pri)
    LIBS += -leng
    LIBS += -lmat
    LIBS += -lhdf5_hl
    LIBS += -lhdf5
    LIBS += -lpthread
    SOURCES += \
        $${PWD}/../../../src/utils/common_system_unix.cpp
}

LIBS += -L$${PWD}/../../../sys/$$CODENAME/lib
LIBS += -lextendbylib_functions

SOURCES += \
    $${PWD}/../../../src/emulator/main_matlabemulator.cpp                       \
    $${PWD}/../../../src/emulator/matlab_emulator_application_c.cpp             \
    $${PWD}/../../../src/emulator/matlab_emulator_mainwindow.cpp                \
    $${PWD}/../../../src/emulator/matlab_emulator_centralwidget.cpp             \
    $${PWD}/../../../src/emulator/matlab_emulator_commandprompt.cpp             \
    $${PWD}/../../../src/croot/daq_root_reader.cpp                              \
    $${PWD}/../../../src/utils/common_system_common.cpp                         \
    $${PWD}/../../../src/utils/common_createnewprocess_unix.cpp                 \
    $${PWD}/../../../src/utils/pitz_daq_data_memory_base.cpp                    \
    $${PWD}/../../../src/utils/pitz_daq_data_memory_forclient.cpp               \
    $${PWD}/../../../src/utils/pitz_daq_data_entryinfo.cpp                      \
    $${PWD}/../../../src/utils/common_system_handlelib.cpp                      \
    $${PWD}/../../../src/indexing/pitz_daq_data_indexing.cpp                    \
    $${PWD}/../../../src/qt_code_editor/codeeditor.cpp

HEADERS += \
    $${PWD}/../../../src/emulator/matlab_emulator_application_c.hpp             \
    $${PWD}/../../../src/emulator/matlab_emulator_mainwindow.hpp                \
    $${PWD}/../../../src/emulator/matlab_emulator_centralwidget.hpp             \
    $${PWD}/../../../src/emulator/matlab_emulator_commandprompt.hpp             \
    $${PWD}/../../../src/utils/common_system_include_private.hpp                \
    $${PWD}/../../../include/matlab/emulator/extendbyexe_functions.hpp          \
    $${PWD}/../../../include/matlab/emulator/extend1bylib.hpp                   \
    $${PWD}/../../../include/matlab/emulator/extend2bylib.hpp                   \
    $${PWD}/../../../include/matlab/emulator/extendbylib_common.hpp             \
    $${PWD}/../../../include/matlab/emulator/extendbylib_print_functions.hpp    \
    $${PWD}/../../../include/matlab/emulator/extendbylib_other_functions.hpp    \
    $${PWD}/../../../src/include_p/extendbylib_functions_private.hpp            \
    $${PWD}/../../../include/common/system/runexe.hpp                           \
    $${PWD}/../../../include/common/system/handlelib.hpp                        \
    $${PWD}/../../../include/common/matlabemulator_compiler_internal.h          \
    $${PWD}/../../../include/daq_root_reader.hpp                                \
    $${PWD}/../../../include/pitz/daq/data/memory/base.hpp                      \
    $${PWD}/../../../include/pitz/daq/data/memory/forclient.hpp                 \
    $${PWD}/../../../include/pitz/daq/data/entryinfo.hpp                        \
    $${PWD}/../../../include/pitz/daq/data/indexing.hpp                         \
    $${PWD}/../../../src/qt_code_editor/codeeditor.hpp


OTHER_FILES += \
    $${PWD}/../../../src/utils/pitz_daq_data_memory_forserver.cpp               \
    $${PWD}/../../../include/pitz/daq/data/memory/forserver.hpp
