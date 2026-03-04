#!/bin/sh
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot

cp sysroot/boot/bados.kernel isodir/boot/bados.kernel
cp sysroot/ramdisk.tar isodir/boot/ramdisk.tar

cat > isodir/limine.cfg << EOF
TIMEOUT=0
:MeatOS
PROTOCOL=limine
KERNEL_PATH=boot:///boot/bados.kernel
MODULE_PATH=boot:///boot/ramdisk.tar
EOF

cp limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin isodir/
mkdir -p isodir/EFI/BOOT
cp limine/BOOTX64.EFI isodir/EFI/BOOT/
cp limine/BOOTIA32.EFI isodir/EFI/BOOT/

xorriso -as mkisofs -b limine-bios-cd.bin \
	-no-emul-boot -boot-load-size 4 -boot-info-table \
	--efi-boot limine-uefi-cd.bin \
	-efi-boot-part --efi-boot-image --protective-msdos-label \
	isodir -o bados.iso

./limine/limine bios-install bados.iso

