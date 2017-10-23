#!/bin/bash

make clean
if [ $HOSTNAME == "rd1-4" ] ; then
  make -j40 |& tee log_of_make.txt

else
  make -j8 |& tee log_of_make.txt
fi

export PREBUILD_BIN="${_WORK_PATH}/projects/vca/tracker_vadp/apps/prebuild_bin/${PLATFORM_SOC}"
echo ${PREBUILD_BIN}
mkdir -p ${PREBUILD_BIN}
cp ./genreport ${PREBUILD_BIN}
