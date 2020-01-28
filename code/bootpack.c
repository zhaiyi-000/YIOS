#include "head.h"

struct BootInfo {
	char CYLS,LEDS;
	short VMODE,SCRNX,SCRNY;
	unsigned char * VRAM;
};


void init_palette();
void init_screen();
void init_gdtidt(void);
void init_mouse_cursor8(char *mouse,int bc);


void putfont8_asc(unsigned char *vram, int xsize, int x, int  y, int c, char *s); //显示字符串
void putblock8_8(unsigned char *vram,int vxsize,int pxsize,int pysize,
	int px0,int py0,char *buf,int bxsize);  //把鼠标的buff写到显存中


void HariMain(){

	struct BootInfo *bInfo = (struct BootInfo *)0xff0;
	unsigned char *vram = bInfo->VRAM;
	int xsize = bInfo->SCRNX;
	int ysize = bInfo->SCRNY;

	init_gdtidt();
	init_palette();
	init_screen(vram, xsize, ysize);

	//logo
	putfont8_asc(vram, xsize,8,8,COL8_RED,"HELLO YIOS");
	putfont8_asc(vram, xsize,7,7,COL8_RED,"HELLO YIOS");

	//debugger
	char s[40];
	sprintf(s, "xsize=%d|",xsize);
	putfont8_asc(vram, xsize,8,32,COL8_RED, s);

	//鼠标
	char mouse[256];
	init_mouse_cursor8(mouse,COL8_RED);
	putblock8_8(vram,xsize,16,16,160,100,mouse,16);

	for(;;){
		io_hlt();
	}
}




