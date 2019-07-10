

NEW_GCC_ROOT__PRIV=/afs/ifh.de/group/pitz/doocs/develop/kalantar/opt/gcc/7.3.0

export COMPILER_PATH=${NEW_GCC_ROOT__PRIV}/bin
export LIBRARY_PATH=${NEW_GCC_ROOT__PRIV}/lib64
export CPLUS_INCLUDE_PATH=${NEW_GCC_ROOT__PRIV}/include/c++/7.3.0
export PATH=${NEW_GCC_ROOT__PRIV}/bin:$PATH


unset NEW_GCC_ROOT__PRIV
