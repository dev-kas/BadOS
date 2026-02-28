#!/bin/sh
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/meat.kernel isodir/boot/meat.kernel
cp sysroot/ramdisk.tar isodir/boot/ramdisk.tar

cat > isodir/boot/grub/grub.cfg << EOF
set default=0
set timeout=0
set gfxpayload=keep
menuentry "meat" {
	multiboot /boot/meat.kernel
	module /boot/ramdisk.tar
}
EOF
grub-mkrescue -o meat.iso isodir
