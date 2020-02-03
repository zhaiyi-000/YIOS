#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;



void yiPrintf(char *chs){
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(bInfo->vram, bInfo->scrnx, COL8_RED, 0, 20, 310, 36);
    putfonts8_asc(bInfo->vram, bInfo->scrnx, 0, 20, COL8_YELLOW, chs);
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

void HariMain(){

    char s[100];
	struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
	int xsize = bInfo->scrnx;
	int ysize = bInfo->scrny;
    char keyBuf__[32];
    char mouseBuf__[128];
    fifo8_init(&keyfifo, 32, keyBuf__);
    fifo8_init(&mousefifo, 128, mouseBuf__);
    
    
    // 检查内存
    struct MEMMAN *memman = (struct MEMMAN *)0x3c0000;  //#define MEMMAN_ADDR 0x3c0000
    unsigned int memtotal = memtest(0x400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x1000, 0x9e000);
    memman_free(memman, 0x400000, memtotal-0x400000);
    
    //初始化图册相关
    struct SHTCTL *shtctl;
    struct SHEET *sht_back,*sht_mouse,*sht_win;
    unsigned char *buf_back,buf_mouse[256],*buf_win;
    shtctl = shtctl_init(memman, bInfo->vram, bInfo->scrnx, bInfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, bInfo->scrnx* bInfo->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160*68);
    sheet_setbuf(sht_back, buf_back, bInfo->scrnx, bInfo->scrny, -1);
    sheet_setbuf(sht_win, buf_win, 160, 68, -1);
    sheet_setbuf(sht_mouse, buf_mouse,  16, 16, 99);
    
    make_window8(buf_win, 160, 68, "window");
    
    putfonts8_asc(buf_win, 160, 0, 30, COL8_RED, "welcom to");
    putfonts8_asc(buf_win, 160, 0, 50, COL8_RED, "YIOS");
    
    sheet_updown(shtctl, sht_back, 0);
    sheet_updown(shtctl, sht_mouse, 2);
    sheet_updown(shtctl, sht_win, 1);

    init_gdtidt();
    init_pic();
    io_sti();   //这个地方产生了一个bug,调试了好久.....
	init_palette();
	init_screen8(buf_back, xsize, ysize);

    
    struct MOUSE_DEC mdec;
    init_keyboard();
    enable_mouse(&mdec);
    
	//logo
    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 0, 310, 18);
	putfonts8_asc(buf_back, xsize,1,1,COL8_YELLOW,"HELLO YIOS");
	putfonts8_asc(buf_back, xsize,0,0,COL8_YELLOW,"HELLO YIOS");
    sheet_refresh(shtctl, sht_back, 0, 0, 310, 18);


	//鼠标
//	char mouse[256];
    int mx = 160,my = 100;
	init_mouse_cursor8(buf_mouse,99);
    
    
    // free 29304=632k(1m-4k(0开头的BIOS)-4k(后面的BIOS)-384k(0xa0000-0xaffff  显存用的地方 64k  后面的320我就不知道是干啥的了))+28m
    sprintf(s, "[total %dM, free %dK]",memtotal/1024/1024,memman_total(memman)/1024);
    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 100, 310, 115);
    putfonts8_asc(buf_back, bInfo->scrnx, 0, 100, COL8_YELLOW, s);
    sheet_refresh(shtctl, sht_back, 0, 100, 310, 115);
    

    sheet_slide(shtctl, sht_back, 0, 0);
    sheet_slide(shtctl, sht_mouse, mx, my);
    sheet_slide(shtctl, sht_win, 80, 72);
    
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
                
                struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
                boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 40, 310, 56);
                putfonts8_asc(buf_back, bInfo->scrnx, 0, 40, COL8_YELLOW, s);
                sheet_refresh(shtctl, sht_back, 0, 40, 310, 56);
                
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
                    
                    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
                    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 60, 310, 76);
                    putfonts8_asc(buf_back, bInfo->scrnx, 0, 60, COL8_YELLOW, s);
                    sheet_refresh(shtctl, sht_back, 0, 60, 310, 76);
                    
                    mx+=mdec.x;
                    my+=mdec.y;
                    
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > bInfo->scrnx - 1) {
                        mx = bInfo->scrnx - 1;
                    }
                    if (my > bInfo->scrny - 1) {
                        my = bInfo->scrny - 1;
                    }
                    
                    sprintf(s, "[zuobiao %3d %3d]",mx,my);
                    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 80, 310, 96);
                    putfonts8_asc(buf_back, bInfo->scrnx, 0, 80, COL8_YELLOW, s);
                    sheet_refresh(shtctl, sht_back, 0, 80, 310, 96);
                    
                    sheet_slide(shtctl, sht_mouse, mx, my);
                }
            }
        }
	}
    
    
}



void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };
    int x, y;
    char c;
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
    boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
    putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
    for (y = 0; y < 14; y++) {
        for (x = 0; x < 16; x++) {
            c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_848484;
            } else if (c == 'Q') {
                c = COL8_C6C6C6;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}
