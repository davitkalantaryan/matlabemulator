#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 Specify the executable path to remove dependency"
  exit 1
fi

patchelf --remove-needed libCore.${RootExtWithVersion}			${1}
patchelf --remove-needed libCint.${RootExtWithVersion}			${1}
patchelf --remove-needed libRIO.${RootExtWithVersion}			${1}
patchelf --remove-needed libNet.${RootExtWithVersion}			${1}
patchelf --remove-needed libHist.${RootExtWithVersion}			${1}
patchelf --remove-needed libGraf.${RootExtWithVersion}			${1}
patchelf --remove-needed libGraf3d.${RootExtWithVersion}		${1}
patchelf --remove-needed libGpad.${RootExtWithVersion}			${1}
patchelf --remove-needed libTree.${RootExtWithVersion}			${1}
patchelf --remove-needed libRint.${RootExtWithVersion}			${1}
patchelf --remove-needed libPostscript.${RootExtWithVersion}	${1}
patchelf --remove-needed libMatrix.${RootExtWithVersion}		${1}
patchelf --remove-needed libPhysics.${RootExtWithVersion}		${1}
patchelf --remove-needed libMathCore.${RootExtWithVersion}		${1}
patchelf --remove-needed libThread.${RootExtWithVersion}		${1}

