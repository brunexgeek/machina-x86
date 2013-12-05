#!/bin/bash

build/tools/mkdfs -d build/install/BOOTIMG.BIN -b build/install/boot/cdemboot.bin -l build/install/boot/osloader.bin -k build/install/boot/osloader.bin -c 512 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs && \
genisoimage -J -f -c BOOTCAT.BIN -b BOOTIMG.BIN -o test.iso build/install && \
rm build/install/BOOTIMG.BIN
