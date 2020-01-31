#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

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


void wait_KBC_sendready(void){
    for (; ; ) {
        if ((io_in8(0x64)&0x2)==0) {
            break;
        }
    }
}

        
void init_keyboard(void){
    wait_KBC_sendready();
    io_out8(0x64, 0x60);
    wait_KBC_sendready();
    io_out8(0x60, 0x47);
}

void enable_mouse(struct MOUSE_DEC *mdec) {
    wait_KBC_sendready();
    io_out8(0x64, 0xd4);
    wait_KBC_sendready();
    io_out8(0x60, 0xf4);  // 0x64是控制和状态端口,0x60是数据端口
    mdec->phase = 0;
}


int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data){
    if (mdec->phase==0) {
        if (data==0xfa) {
            mdec->phase=1;
        }
        return 0;
    }else if(mdec->phase==1){
        if ((data & 0xc8)==0x08) {
            mdec->phase=2;
            mdec->buf[0] = data;
        }
        return 0;
    }else if(mdec->phase==2){
        mdec->phase=3;
        mdec->buf[1] = data;
        return 0;
    }else if(mdec->phase==3){
        mdec->phase=1;
        mdec->buf[2] = data;
        
        mdec->btn = mdec->buf[0] & 0x7;
        mdec->x = (char)mdec->buf[1];
        mdec->y = (char)mdec->buf[2];
        mdec->y = -mdec->y;
        return 1;
    }else{
        return -1;
    }
}
