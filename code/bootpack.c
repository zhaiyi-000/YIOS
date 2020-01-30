#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain(){

	struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
	unsigned char *vram = bInfo->VRAM;
	int xsize = bInfo->SCRNX;
	int ysize = bInfo->SCRNY;

    init_gdtidt();
    init_pic();
    io_sti();   //这个地方产生了一个bug,调试了好久.....
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

    char data;
	for(;;){
        io_cli();
        if (keybuf.len==0) {
            io_stihlt();
        }else{
            data = keybuf.data[keybuf.left];
            keybuf.left++;
            if (keybuf.left==KEYBUFLEN) {
                keybuf.left = 0;
            }
            keybuf.len--;
            io_sti();
            sprintf(s, "%02X",data);
            
            struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
            boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 0, 40*8-1, 15);
            putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 0, COL8_YELLOW, s);
        }
	}
    
    
}




