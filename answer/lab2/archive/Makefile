all:
	cd rootfs&&find . | cpio -o -H newc > ../initramfs.cpio

clean:
	-rm *.cpio