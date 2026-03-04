#!/bin/sh
set -e
. ./iso.sh

echo Launching QEMU...
qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 2G -cdrom meat.iso -serial stdio # -d int
