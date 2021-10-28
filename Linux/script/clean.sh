#!/bin/bash

PRJ_DIR=project
DEM_DIR=demo
LIB_DIR=lib
NDK_PRJ_DIR=$PRJ_DIR/android-ndk
NDK_DEMO_DIR=$DEM_DIR/Release
RMV_ALL=0
DEST_BASE=$(cd "$(dirname "$0")/.."; pwd)

for var in "$@"; do
    if [ $var == "-a" ] || [ $var == "--all" ]; then
        RMV_ALL=1
        break
    fi
done

do_remove()
{
  find "$1" -name "$2" | xargs rm -rf
}

cd $DEST_BASE

do_remove $PRJ_DIR .vs
do_remove $PRJ_DIR Debug
do_remove $PRJ_DIR obj
do_remove $DEM_DIR Debug
do_remove $DEM_DIR obj
do_remove $DEM_DIR .vs
do_remove $LIB_DIR Debug
do_remove $LIB_DIR obj
do_remove $NDK_PRJ_DIR obj
do_remove $NDK_PRJ_DIR libs
do_remove $NDK_DEMO_DIR android-ndk

do_remove . *.sdf
do_remove . *.VC.db
do_remove . *.VC.db-shm
do_remove . *.VC.db-wal
do_remove . *.cki

if [ $RMV_ALL -eq 1 ]; then
    do_remove $LIB_DIR "*.so.*"
fi
