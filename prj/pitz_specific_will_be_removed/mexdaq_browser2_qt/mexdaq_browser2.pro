# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

QMAKE_CXXFLAGS = -fPIC
QMAKE_CFLAGS = -fPIC
QMAKE_LFLAGS = -Wl,-E -pie -ldl

win32{

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
}

include ($${PWD}/../../../contrib/matlab/prj/common/common_qt/matlab_matrix_common.pri)

message("!!!!!!!!!!! CODENAME=$$CODENAME")

# Root configuration
MYROOT_SYS_DIR = $$system(env | grep ROOT_SYS_DIR)
equals($$MYROOT_SYS_DIR,"") {
    MYROOT_SYS_DIR = /afs/ifh.de/group/pitz/data/ers/sys/$${CODENAME}/opt/root/current
    message("!!! MYROOT_SYS_DIR set in the project file: $$MYROOT_SYS_DIR")
} else {
    message("!!! MYROOT_SYS_DIR comes from environment: $$MYROOT_SYS_DIR")
}
QMAKE_CXXFLAGS += $$system($$MYROOT_SYS_DIR/bin/root-config --cflags)
DEFINES += R__NULLPTR

#DEFINES += USE_REAL_ROOT
#LIBS += $$system($$MYROOT_SYS_DIR/bin/root-config --libs)
LIBS += -lroot_for_matlab_false
LIBS += -L$${PWD}/../libroot_for_matlab_false_qt


# message("ROOTCFLAGS=$$ROOTCFLAGS")
# INCLUDEPATH += $$MYROOT_SYS_DIR/include

# C++ 11 story
#optionsCpp11 = $$find(CONFIG, "cpp11")
#count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x
##DEFINES += nullptr=NULL
#QMAKE_CXXFLAGS += -std=c++0x

#include(../../common/common_qt/mex_common.pri)

#TARGET_EXT = mexa64
TARGET = mexdaq_browser2.mexa64
#TEMPLATE = lib

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -L$${PWD}/../../../sys/$$CODENAME/lib
CONFIG += debug
QT -= core
QT -= gui


#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
#INCLUDEPATH += ../../../include
#INCLUDEPATH += ../../../contrib/cpp-raft/include

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../src/pitz_specific_will_be_removed
INCLUDEPATH += $${PWD}/../../../include/pitz_specific_will_be_removed

SOURCES += \
    $${PWD}/../../../src/pitz_specific_will_be_removed/mainentry_daq_browser2.cpp                  \
    $${PWD}/../../../src/pitz_specific_will_be_removed/mexentry_daq_browser2.cpp

HEADERS += \
    $${PWD}/../../../src/pitz_specific_will_be_removed/pitz_daq_root_for_matlab_false.hpp          \
    $${PWD}/../../../src/pitz_specific_will_be_removed/mex_daq_browser2_common.h                   \
    $${PWD}/../../../include/pitz_specific_will_be_removed/daq_root_reader.hpp

OTHER_FILES += \
    $${PWD}/../../../src/pitz_specific_will_be_removed/.old/entry_mex_daq_browser2.cpp
