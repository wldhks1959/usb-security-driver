sudo mount /home/jiwan/proj_usb/buildroot/output/images/rootfs.ext4 /mnt/qemu_rootfs
sudo cp /home/jiwan/proj_usb/user_src/test_ioctl /mnt/qemu_rootfs/usr/local/bin/
sync
sudo umount /mnt/qemu_rootfs
