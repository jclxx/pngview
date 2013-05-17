pngview: bitstream.h deflate.h gui.h huffman.h png.h \
	zlib.h bitstream.c deflate.c gui.c huffman.c main.c png.c zlib.c
	cc -O3 -Wall -o pngview -std=c99 *.c -lm -lX11
