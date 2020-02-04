#include "bootpack.h"
extern struct TIMECTL timerctl;

struct FIFO32 fifo;

void yiPrintf(char *chs){
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(bInfo->vram, bInfo->scrnx, COL8_RED, 0, 20, 310, 36);
    putfonts8_asc(bInfo->vram, bInfo->scrnx, 0, 20, COL8_YELLOW, chs);
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c,int b, char*s, int l);

void HariMain(){

    struct TIMER *timer;
    
    char s[100];
    int fifobuf[128];
	struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
	int xsize = bInfo->scrnx;
	int ysize = bInfo->scrny;
    unsigned int memtotal;
    struct MEMMAN *memman;
    struct MOUSE_DEC mdec;
    
    //初始化图册相关
    struct SHTCTL *shtctl;
    struct SHEET *sht_back,*sht_mouse,*sht_win;
    unsigned char *buf_back,buf_mouse[256],*buf_win;
    
    
    fifo32_init(&fifo, 128, fifobuf);
    
    
    // 检查内存
    memman = (struct MEMMAN *)0x3c0000;  //#define MEMMAN_ADDR 0x3c0000
    memtotal = memtest(0x400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x1000, 0x9e000);
    memman_free(memman, 0x400000, memtotal-0x400000);
    
    
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
    
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);
    

    init_gdtidt();
    init_pic();
    io_sti();   //这个地方产生了一个bug,调试了好久.....
	init_palette();
	init_screen8(buf_back, xsize, ysize);
    init_pit();

    io_out8(PIC0_IMR, 0xf8);
    io_out8(PIC1_IMR, 0xef);
    
    init_keyboard(&fifo,256);
    enable_mouse(&fifo,256,&mdec);
    
	//logo
    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 0, 310, 18);
	putfonts8_asc(buf_back, xsize,1,1,COL8_YELLOW,"HELLO YIOS");
	putfonts8_asc(buf_back, xsize,0,0,COL8_YELLOW,"HELLO YIOS");
    sheet_refresh(sht_back, 0, 0, 310, 18);


	//鼠标
    int mx = 160,my = 100;
	init_mouse_cursor8(buf_mouse,99);
    
    
    // free 29304=632k(1m-4k(0开头的BIOS)-4k(后面的BIOS)-384k(0xa0000-0xaffff  显存用的地方 64k  后面的320我就不知道是干啥的了))+28m
    sprintf(s, "[total %dM, free %dK]",memtotal/1024/1024,memman_total(memman)/1024);
    putfonts8_asc_sht(sht_back, 0, 100, COL8_YELLOW, COL8_RED, s, 30);
    

    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_mouse, mx, my);
    sheet_slide(sht_win, 80, 72);
    
    timer = timer_alloc();
    timer_init(timer, &fifo, 0);
    timer_settime(timer,50);
    
    int data;
	for(;;){
        
        io_cli();
        if (fifo32_status(&fifo) ==0) {
            io_sti();
        }else{
            
            data = fifo32_get(&fifo);
            io_sti();
            
            if (256 <= data && data <= 511) {
                data-=256;
                
                sprintf(s, "jianpan %02X",data);
                putfonts8_asc_sht(sht_back, 0, 40, COL8_YELLOW, COL8_RED, s, 20);
                
            }else if(512 <= data && data <= 767){
                data-=512;
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
                    
                    putfonts8_asc_sht(sht_back, 0, 60, COL8_YELLOW, COL8_RED, s, 30);
                    
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
                    putfonts8_asc_sht(sht_back, 0, 80, COL8_YELLOW, COL8_RED, s, 20);
                    
                    sheet_slide( sht_mouse, mx, my);
                }
            }else if(data==0){
                timer_init(timer, &fifo, 1);
                boxfill8(buf_back, xsize, COL8_RED, 0, 20, 310, 36);
                timer_settime(timer, 50);
                sheet_refresh( sht_back, 0, 20, 310, 36);
            }else if(data==1){
                timer_init(timer, &fifo, 0);
                boxfill8(buf_back, xsize, COL8_YELLOW, 0, 20, 310, 36);
                timer_settime(timer, 50);
                sheet_refresh( sht_back, 0, 20, 310, 36);
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

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c,int b, char*s, int l) {
    boxfill8(sht->buf, sht->bxsize, b, x, y, x+8*l-1, y+15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh( sht, x, y, x+l*8, y+16);   //因为里面是 < 不是<= ,所有是16不是15
}
