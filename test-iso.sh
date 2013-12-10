#!/bin/bash

cat build/install/boot/osloader-stub.bin build/install/boot/osloader-main.bin > build/install/boot/osloader.bin && \
build/tools/mkdfs -d build/install/BOOTIMG.BIN -b build/install/boot/cdemboot.bin -l build/install/boot/osloader.bin -k ../sanos/linux/install/boot/krnl.dll -c 512 -C 1440 -I 8192 -i -f -K rootdev=cd0,rootfs=cdfs && \
genisoimage -J -f -c BOOTCAT.BIN -b BOOTIMG.BIN -o test.iso build/install && \
rm build/install/BOOTIMG.BIN
