all:
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse
	gcc -Wall ssd_fuse_dut.c -o ssd_fuse_dut
	mkdir /tmp/ssd

ta: all
	./ssd_fuse -d /tmp/ssd

clean:
	rm -rf /tmp/ssd 
	rm -rf ssd_fuse 
	rm -rf ssd_fuse_dut

fuse:
	fusermount -u /tmp/ssd

	
tb:
	./test.sh test1
	./test.sh test2