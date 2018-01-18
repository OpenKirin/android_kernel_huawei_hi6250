#!/bin/bash

LOCAL_DIR=`pwd`
export PATH="$PATH:/home/xepeleato/AOSPO/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/"
export CROSS_COMPILE="aarch64-linux-android-"
mkdir -p out
echo "Paths and Toolchain loaded!"
make ARCH=arm64 O=./out merge_hi6250_defconfig
make ARCH=arm64 O=./out -j8
printf "\nDone! if it compiled correctly, you'll find the compiled Image at ../out/arch/arm64/boot/Image"
printf "\nThe modules are at out/(device)/*.ko\n"
