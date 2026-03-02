#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done

cp ./ramdisk/* $SYSROOT/ramdisk/
7z a $SYSROOT/ramdisk.tar $SYSROOT/ramdisk/*
