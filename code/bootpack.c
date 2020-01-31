#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;



void yiPrintf(){
    struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 20, 310, 36);
    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 20, COL8_YELLOW, "wo lai~~~~~");
}

void HariMain(){

    char s[100];
	struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
	unsigned char *vram = bInfo->VRAM;
	int xsize = bInfo->SCRNX;
	int ysize = bInfo->SCRNY;
    char keyBuf__[32];
    char mouseBuf__[128];
    fifo8_init(&keyfifo, 32, keyBuf__);
    fifo8_init(&mousefifo, 128, mouseBuf__);

    init_gdtidt();
    init_pic();
    io_sti();   //这个地方产生了一个bug,调试了好久.....
	init_palette();
	init_screen(vram, xsize, ysize);

    
    struct MOUSE_DEC mdec;
    init_keyboard();
    enable_mouse(&mdec);
    
	//logo
    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 0, 310, 18);
	putfont8_asc(vram, xsize,1,1,COL8_YELLOW,"HELLO YIOS");
	putfont8_asc(vram, xsize,0,0,COL8_YELLOW,"HELLO YIOS");


	//鼠标
	char mouse[256];
    int mx = 160,my = 100;
	init_mouse_cursor8(mouse,COL8_008484);
	putblock8_8(vram,xsize,16,16,mx,my,mouse,16);

    unsigned char data;
	for(;;){
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo)==0) {
            io_stihlt();
        }else{
            if (fifo8_status(&keyfifo)!=0) {
                data = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "jianpan %02X",data);
                
                struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
                boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 40, 310, 56);
                putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 40, COL8_YELLOW, s);
            }else if(fifo8_status(&mousefifo)!=0){
                data = fifo8_get(&mousefifo);
                io_sti();
                
                if (mouse_decode(&mdec, data) != 0) {
                    
                    sprintf(s, "lcr %8X %8X",mdec.x,mdec.y);
                    
                    if ((mdec.btn & 0x1)!=0) {
                        s[0] = 'L';
                    }
                    if ((mdec.btn & 0x2)!=0) {
                        s[2] = 'R';
                    }
                    if ((mdec.btn & 0x4)!=0) {
                        s[1] = 'C';
                    }
                    
                    struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 60, 310, 76);
                    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 60, COL8_YELLOW, s);
                    
                    //消除鼠标
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_008484, mx, my, mx+15, my+15);
                    
                    mx+=mdec.x;
                    my+=mdec.y;
                    
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > bInfo->SCRNX - 16) {
                        mx = bInfo->SCRNX - 16;
                    }
                    if (my > bInfo->SCRNY - 16) {
                        my = bInfo->SCRNY - 16;
                    }
                    
                    sprintf(s, "[zuobiao %3d %3d]",mx,my);
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 80, 310, 96);
                    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 80, COL8_YELLOW, s);
                    putblock8_8(bInfo->VRAM, bInfo->SCRNX, 16, 16, mx, my, mouse, 16);
                }
            }
        }
	}
    
    
}
