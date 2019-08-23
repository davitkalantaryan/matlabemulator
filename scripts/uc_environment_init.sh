# 
# file:			uc_environment_init.sh
# created on:		2019 Aug 21
# created bY:		Davit Kalantaryan (davit.kalantaryan@desy.de)
# purpose:		To init environmental variables 
#

init_uc_environment_currentDirectory=`pwd`

#scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" # instead of this
init_uc_environment_scriptDirectoryBase=`dirname ${0}`
cd ${init_uc_environment_scriptDirectoryBase}
init_uc_environment_fileOrigin=`readlink ${0}`
if [ ! -z "$init_uc_environment_fileOrigin" ]; then
	init_uc_environment_relativeSourceDir=`dirname ${init_uc_environment_fileOrigin}`
	cd ${init_uc_environment_relativeSourceDir}
	unset init_uc_environment_relativeSourceDir
fi
unset init_uc_environment_fileOrigin
init_uc_environment_scriptDirectory=`pwd`
cd ${init_uc_environment_currentDirectory}

init_uc_environment_fileOriginlsbCode=`lsb_release -c | cut -f 2`


echo "scriptDirectory="${init_uc_environment_scriptDirectory}



CROOT_DIRECTOR_ROOT__PRIV=/afs/ifh.de/amd64_rhel50/products/root64/5.28.00
CROOT_LD_LIBRARY_PATH=`${CROOT_DIRECTOR_ROOT__PRIV}/bin/root-config --libdir`

. init_qt_for_current_qt

export LD_LIBRARY_PATH=/afs/ifh.de/SL/6/x86_64/opt/matlab/R2012b/bin/glnxa64:$CROOT_LD_LIBRARY_PATH:$LD_LIBRARY_PATH:/afs/ifh.de/group/pitz/doocs/data/ers/sys/Nitrogen/opt/gcc/7.3.0/lib64 
export PATH=/afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/bin:$PATH

unset CROOT_LD_LIBRARY_PATH
unset CROOT_DIRECTOR_ROOT__PRIV

unset init_uc_environment_currentDirectory
unset init_uc_environment_scriptDirectory
