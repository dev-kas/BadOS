#!/bin/sh
set -e
. ./iso.sh

echo Launching QEMU...
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom meat.iso -serial stdio
