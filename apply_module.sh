# mount 
sudo mount /home/jiwan/proj_usb/buildroot/output/images/rootfs.ext4 /mnt/qemu_rootfs
# module cp
sudo cp drivers/usb_security/main.ko /mnt/qemu_rootfs/lib/modules/6.17.0-rc1-gd7ee5bdce789-dirty/kernel/drivers/usb_security/
sync
sudo umount /mnt/qemu_rootfs
