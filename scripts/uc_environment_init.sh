

CROOT_DIRECTOR_ROOT__PRIV=/afs/ifh.de/amd64_rhel50/products/root64/5.28.00


CROOT_LD_LIBRARY_PATH=`${CROOT_DIRECTOR_ROOT__PRIV}/bin/root-config --libdir`

. /afs/ifh.de/group/pitz/doocs/data/ers/scripts/unix/init_qt_for_qt5

export LD_LIBRARY_PATH=/afs/ifh.de/SL/6/x86_64/opt/matlab/R2012b/bin/glnxa64:$CROOT_LD_LIBRARY_PATH:$LD_LIBRARY_PATH
export PATH=/afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/bin:$PATH

unset CROOT_LD_LIBRARY_PATH
unset CROOT_DIRECTOR_ROOT__PRIV
