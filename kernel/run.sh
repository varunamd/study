#/bin/sh


export KERNEL=kernel_2712
make bcm2712_defconfig

make -j4 Image.gz modules dtbs
sudo make modules_install
sudo cp arch/arm64/boot/dts/broadcom/*.dtb /boot/firmware/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* /boot/firmware/overlays/
sudo cp arch/arm64/boot/dts/overlays/README /boot/firmware/overlays/
sudo cp arch/arm64/boot/Image.gz /boot/firmware/$KERNEL.img
