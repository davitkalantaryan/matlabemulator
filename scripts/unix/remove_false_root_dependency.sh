#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 Specify the executable path to remove dependency"
  exit 1
fi


patchelf --remove-needed libroot_for_matlab_false.so.1		${1}

