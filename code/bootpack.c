#include "bootpack.h"
extern struct TIMECTL timerctl;

struct FIFO32 fifo;
struct TSS32 {
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};
int task_b_esp;

void yiPrintf(char *chs){
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(bInfo->vram, bInfo->scrnx, COL8_RED, 0, 120, 310, 136);
    putfonts8_asc(bInfo->vram, bInfo->scrnx, 0, 120, COL8_YELLOW, chs);
}

void task_b_main() {
    
    struct TIMER *timer;
    struct FIFO32 fifo;
    int data;
    
    int fifobuf[128];
    fifo32_init(&fifo, 128, fifobuf);
    
    timer = timer_alloc();
    timer_init(timer, &fifo, 5);
    timer_settime(timer, 500);
    
    
    for (; ; ) {
        io_cli();
        if (fifo32_status(&fifo)==0) {
            io_stihlt();
        }else{
            data = fifo32_get(&fifo);
            io_sti();
            if (data==5) {
                yiPrintf("task3~~~");
                taskswitch3();
            }
        }
        
    }
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c,int b, char*s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

void HariMain(){
    
    // 检查内存
    struct MEMMAN *memman = (struct MEMMAN *)0x3c0000;  //#define MEMMAN_ADDR 0x3c0000
    unsigned int memtotal = memtest(0x400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x1000, 0x9e000);
    memman_free(memman, 0x400000, memtotal-0x400000);

    struct TIMER *timer,*timer5;
    
    char s[100];
    int fifobuf[128];
	struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
	int xsize = bInfo->scrnx;
	int ysize = bInfo->scrny;
    
    
    struct MOUSE_DEC mdec;
    
    //初始化图册相关
    struct SHTCTL *shtctl;
    struct SHEET *sht_back,*sht_mouse,*sht_win;
    unsigned char *buf_back,buf_mouse[256],*buf_win;
    int cursor_x;
    int data;
    
    static char keytable[0x54] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.'
    };
    
    struct TSS32 tss_a,tss_b;
    tss_a.ldtr = 0;
    tss_a.iomap = 0x40000000;  //别问为什么  p293
    tss_b.ldtr = 0;
    tss_b.iomap = 0x40000000;
    
    tss_b.eip = (int)&task_b_main;
    tss_b.eflags = 0x202;   //if = 1
    tss_b.eax = 0;
    tss_b.ecx = 0;
    tss_b.edx = 0;
    tss_b.ebx = 0;
    task_b_esp = memman_alloc_4k(memman, 64*1024)+64*1024;
    tss_b.esp = task_b_esp;
    tss_b.ebp = 0;
    tss_b.esi = 0;
    tss_b.edi = 0;
    tss_b.es = 1*8;
    tss_b.cs = 2*8;
    tss_b.ss = 1*8;
    tss_b.ds = 1*8;
    tss_b.fs = 1*8;
    tss_b.gs = 1*8;
    
    
    
    fifo32_init(&fifo, 128, fifobuf);
    
    
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
    
//    putfonts8_asc(buf_win, 160, 0, 30, COL8_RED, "welcom to");
//    putfonts8_asc(buf_win, 160, 0, 50, COL8_RED, "YIOS");
    make_textbox8(sht_win, 10, 30, sht_win->bxsize-20, 16, COL8_WHITE);
    
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
    enable_mouse(&fifo,512,&mdec);
    
    // dgt/idt 增设
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    set_segmdesc(gdt+3, 103, (int)&tss_a, AR_TSS32);
    set_segmdesc(gdt+4, 103, (int)&tss_b, AR_TSS32);
    load_tr(3*8);
    
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
    sheet_slide(sht_win, 80, 172);
    
    timer = timer_alloc();
    timer_init(timer, &fifo, 0);
    timer_settime(timer,50);
    
    timer5 =timer_alloc();
    timer_init(timer5, &fifo, 5);
    timer_settime(timer5,500);
    
    cursor_x = 8;
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
                
                if (data < 0x54 && keytable[data]!=0) {
                    s[0] = keytable[data];
                    s[1] = 0;
                    putfonts8_asc_sht(sht_win, cursor_x, 30, COL8_RED, COL8_YELLOW, s, 1);
                    cursor_x+=8;
                }
                
                if (data==0xe && cursor_x > 8) {
                    boxfill8(buf_win, sht_win->bxsize, COL8_WHITE, cursor_x, 30, cursor_x+2, 46);
                    sheet_refresh( sht_win, cursor_x, 30, cursor_x+2, 46);
                    cursor_x -= 8;
                    putfonts8_asc_sht(sht_win, cursor_x, 30, COL8_WHITE, COL8_WHITE, " ", 1);
                }
                
                
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
                    
                    if ((mdec.btn & 0x01) != 0) {
                        /* 左ボタンを押していたら、sht_winを動かす */
                        sheet_slide(sht_win, mx - 80, my - 8);
                    }
                }
            }else if(data==0){
                timer_init(timer, &fifo, 1);
                boxfill8(buf_win, sht_win->bxsize, COL8_BLACK, cursor_x, 30, cursor_x+2, 46);
                timer_settime(timer, 50);
                sheet_refresh( sht_win, cursor_x, 30, cursor_x+2, 46);
            }else if(data==1){
                timer_init(timer, &fifo, 0);
                boxfill8(buf_win, sht_win->bxsize, COL8_WHITE, cursor_x, 30, cursor_x+2, 46);
                timer_settime(timer, 50);
                sheet_refresh( sht_win, cursor_x, 30, cursor_x+2, 46);
            }else if(data==5){
                putfonts8_asc_sht(sht_back, 0, 140, COL8_YELLOW, COL8_RED, "[5s]", 20);
                taskswitch4();
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


void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
    int x1 = x0 + sx, y1 = y0 + sy;
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
    return;
}
