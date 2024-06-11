#!/bin/bash

/mnt/sdc/outside_home/asgard/qemu/build/qemu-system-aarch64 -smp 2 \
  -M virt -cpu max -m 1G \
  -boot menu=on \
  -drive file=QEMU_EFI.qcow2,format=qcow2,if=pflash,readonly=on \
  -drive file=QEMU_VARS.qcow2,format=qcow2,if=pflash,readonly=on \
  -drive if=virtio,file=root.qcow2,format=qcow2 \
  -netdev user,id=unet,hostfwd=tcp::2244-:22 \
  -device virtio-net-device,netdev=unet \
  -nographic -no-reboot -loadvm base1

# In case you want to install the system, using the following line.
#  -drive if=virtio,format=raw,file=alpine-virt-3.18.2-aarch64.iso \
