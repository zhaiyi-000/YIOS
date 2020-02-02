#include "bootpack.h"

void init_palette(){
	static unsigned char rgb[16*3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};

	set_palette(0,15,rgb);
}

void set_palette(int start, int end, unsigned char *rgb) {
	int i;
	int flag = io_load_eflags();
	io_cli();
	io_out8(0x3c8,start);
	for(i = start;i<=end;i++){
		io_out8(0x3c9,rgb[0]/4);
		io_out8(0x3c9,rgb[1]/4);
		io_out8(0x3c9,rgb[2]/4);
		rgb+=3;
	}
	io_store_eflags(flag);
}

void init_screen8(unsigned char *vram, int xsize, int ysize) {

	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
}

void boxfill8(unsigned char *vram,int xsize,int c,int x0,int y0,int x1,int y1){
	int x,y;
	for (y=y0;y<=y1;y++) {
		for (x=x0;x<=x1;x++) {
			vram[y*xsize+x] = c;
		}
	}
}

//显示字符
void putfont8(unsigned char *vram, int xsize, int x, int y, int c, char *font) {
	int i;
	char *p, ch;  //bug:把p申明成了 int*,导致显示不正常
	for (i = 0; i < 16; ++i) {
		p = vram+(y+i)*xsize+x;
		ch = font[i];
		if((ch & 0x80) !=0) p[0] = c;
		if((ch & 0x40) !=0) p[1] = c;
		if((ch & 0x20) !=0) p[2] = c;
		if((ch & 0x10) !=0) p[3] = c;
		if((ch & 0x08) !=0) p[4] = c;
		if((ch & 0x04) !=0) p[5] = c;
		if((ch & 0x02) !=0) p[6] = c;
		if((ch & 0x01) !=0) p[7] = c;
	}
}

void putfonts8_asc(unsigned char *vram, int xsize, int x, int  y, int c, char *s) {
	extern char hankaku[4096];
	for (;*s!=0;s++) {
		putfont8(vram,xsize,x,y,c,hankaku+(*s)*16);
		x+=8;
	}
}

void init_mouse_cursor8(char *mouse,int bc) {
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int idx,x,y;

	for (y = 0; y < 16; ++y) {
		for (x = 0; x < 16; ++x) {
			idx = y*16+x;
			if (cursor[y][x]=='.') mouse[idx] = bc;
			else if (cursor[y][x]=='*') mouse[idx] = COL8_BLACK;
			else mouse[idx] = COL8_WHITE;
		}
	}
}

void putblock8_8(unsigned char *vram,int vxsize,int pxsize,int pysize,
	int px0,int py0,char *buf,int bxsize) {
	int x,y;
	for (y = 0; y < pysize; ++y) {
		for (x = 0; x < pxsize; ++x) {
			vram[(y+py0)*vxsize+x+px0] = buf[y*bxsize+x];
		}
	}
}
