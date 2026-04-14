#!/bin/sh
set -e
. ./iso.sh

echo Launching QEMU...
qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 154M -cdrom bados.iso -serial stdio # -d int
