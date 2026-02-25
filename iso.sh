#!/bin/sh
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/bin/meat.kernel isodir/boot/meat.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "meat" {
	multiboot /boot/meat.kernel
}
EOF
grub-mkrescue -o meat.iso isodir
