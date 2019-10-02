#
# file:         matlabemulator_workspace.pro
# created on:   2019 Sep 04
#

win32{

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
}
message("OUTPUT_DIRCETORY=" $$OUTPUT_DIRCETORY)

TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = $${OUT_PWD}

SUBDIRS += \
    $${PWD}/../../prj/pitz_specific_will_be_removed/libroot_for_matlab_false_qt/root_for_matlab_false.pro       \
    $${PWD}/../../prj/pitz_specific_will_be_removed/mexdaq_browser2_qt/mexdaq_browser2.pro                      \
    $${PWD}/../../prj/pitz_specific_will_be_removed/test_mexhacking_qt/test_mexhacking.pro                      \
    $${PWD}/../../prj/pitz_specific_will_be_removed/test_mex_root_crash_qt/test_mex_root_crash.pro              \
    $${PWD}/../../prj/emulator/emulator_qt/emulator.pro                                                         \
    $${PWD}/../../prj/libs/extendbyexe_functions_qt/extendbyexe_functions.pro                                   \
    $${PWD}/../../prj/libs/extendbylib_functions_qt/extendbylib_functions.pro                                   \
    $${PWD}/../../prj/tests/libextend1_test_qt/extend1_test.pro                                                 \
    $${PWD}/../../prj/tests/libextend2_test_qt/extend2_test.pro                                                 \
    $${PWD}/../../prj/tests/libextend_with_functions_test_qt/extend_with_functions_test.pro                     \
    $${PWD}/../../prj/tests/prog_with_stdin_test_qt/prog_with_stdin_test.pro



#test/prog_with_stdin_test.subdir = $${PWD}/../../prj/tests/prog_with_stdin_test_qt/prog_with_stdin_test.pro
# system(export OUTPUT_DIRCETORY=$$OUTPUT_DIRCETORY)

$${PWD}/../../prj/pitz_specific_will_be_removed/mexdaq_browser2_qt/mexdaq_browser2.pro.depends = \
    $${PWD}/../../prj/pitz_specific_will_be_removed/libroot_for_matlab_false_qt/root_for_matlab_false.pro
$${PWD}/../../prj/pitz_specific_will_be_removed/test_mexhacking_qt/test_mexhacking.pro.depends = \
    $${PWD}/../../prj/pitz_specific_will_be_removed/libroot_for_matlab_false_qt/root_for_matlab_false.pro


$${PWD}/../../prj/emulator/emulator_qt/emulator.pro.depends = $${PWD}/../../prj/libs/extendbylib_functions_qt/extendbylib_functions.pro
$${PWD}/../../prj/tests/libextend_with_functions_test_qt/extend_with_functions_test.pro.depends = $${PWD}/../../prj/libs/extendbylib_functions_qt/extendbylib_functions.pro

OTHER_FILES += \
    $${PWD}/.qmake.conf
