qemu-system-aarch64 \
	-kernel /home/jiwan/proj_usb/linux/arch/arm64/boot/Image \
	-drive format=raw,file=/home/jiwan/proj_usb/buildroot/output/images/rootfs.ext4,if=virtio \
	-append "root=/dev/vda console=ttyAMA0 nokaslr" \
	-nographic -M virt -cpu cortex-a72 -m 2G -smp 2 \
	-device qemu-xhci,id=xhci \
	-usb -device usb-host,vendorid=0x0781,productid=0x5581 \
