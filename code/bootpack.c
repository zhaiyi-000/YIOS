#include "bootpack.h"

#define KEYCMD_LED 0xed  //键盘指示灯用

struct FILEINFO {
    unsigned char name[8], ext[3], type;
    char reserve[10];
    unsigned short time, date, clustno;
    unsigned int size;
};

void yiPrintf(char *chs){
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(bInfo->vram, bInfo->scrnx, COL8_RED, 0, 120, 310, 136);
    putfonts8_asc(bInfo->vram, bInfo->scrnx, 0, 120, COL8_YELLOW, chs);
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c,int b, char*s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf, int xsize,char *title, char act);
int cons_newline(int cursor_y, struct SHEET *sheet);

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    int x,y;
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct FIFO32 *fifo = &task->fifo;
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG+0x2600);

    int i, fifobuf[128], cursor_x = 16,cursor_y = 28, cursor_c = -1;
    fifo32_init(fifo, 128, fifobuf, task);
    char s[100],cmdline[30];
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

    timer = timer_alloc();
    timer_init(timer, fifo, 1);
    timer_settime(timer, 50);
    
    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">",1);

    for (;;) {
        io_cli();
        if (fifo32_status(fifo) == 0) {
            task_sleep(task);
            io_sti();
        } else {
            i = fifo32_get(fifo);
            io_sti();
            if (i <= 1) { /* カーソル用タイマ */
                if (i != 0) {
                    timer_init(timer, fifo, 0); /* 次は0を */
                    if (cursor_c >= 0) {
                        cursor_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, fifo, 1); /* 次は1を */
                    if (cursor_c >= 0) {
                        cursor_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }else if (i == 2) { //显示光标
                cursor_c = COL8_WHITE;
            }else if (i ==3) { //不显示光标
                boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, cursor_x, cursor_y, cursor_x+7, 43);
                cursor_c = -1;
            }else if (256 <= i && i <=511) {
                i-=256;
                if (i==8) {
                    if (cursor_x > 16) {
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                        cursor_x -= 8;
                    }
                }else if (i==10){
                    putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, " ", 1);
                    cmdline[cursor_x/8-2] = 0;
                    cursor_y = cons_newline(cursor_y, sheet);
                    
                    if (strcmp(cmdline,"212")==0) {
                        sprintf(s, "total %dMB", memtotal/1024/1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        
                        sprintf(s, "free %dKB", memman_total(memman)/1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }else if (strcmp(cmdline,"321")==0) {
                        for (y = 28; y < 28+128; y++) {
                            for (x = 8; x < 8+240; x++) {
                                sheet->buf[x+y*sheet->bxsize] = COL8_BLACK;
                            }
                        }
                        sheet_refresh(sheet, 8, 28, 8+240, 28+128);
                        cursor_y = 28;
                    }else if (strcmp(cmdline,"123")==0) {
                        for (x = 0; x < 224; x++) {
                            if (finfo[x].name[0] == 0x0) {
                                break;
                            }
                            if (finfo[x].name[0]!=0xe5) { //0xe5代表文件删除了
                                if ((finfo[x].type & 0x18)==0) { //非目录,非非文件信息
                                    sprintf(s, "filename.exe    %7d",finfo[x].size);
                                    for (y = 0; y < 8; y++) {
                                        s[y] = finfo[x].name[y];
                                    }
                                    for (y = 0; y < 3; y++) {
                                        s[9+y] = finfo[x].ext[y];
                                    }
                                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
                                    cursor_y = cons_newline(cursor_y, sheet);
                                }
                            }
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                    }else if(cmdline[0]!=0){
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, "Bad command.", 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, ">", 1);
                    cursor_x = 16;
                }else{
                    if (cursor_x < 240) {
                        s[0] = i;
                        s[1] = 0;
                        cmdline[cursor_x/8-2] = i;
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x+=8;
                    }
                }
            }
            
            if (cursor_c >= 0) {
                boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
            }
            sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
        }
    }
}



void HariMain(){
    
    //数据
    static char keytable0[0x80] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
    };
    static char keytable1[0x80] = {
        0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    
    
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[100];
    //缓冲区
    struct FIFO32 fifo;
    int fifobuf[128];
    fifo32_init(&fifo, 128, fifobuf,0);
    
    // 检查内存
    struct MEMMAN *memman = (struct MEMMAN *)0x3c0000;  //#define MEMMAN_ADDR 0x3c0000
    unsigned int memtotal = memtest(0x400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x1000, 0x9e000);
    memman_free(memman, 0x400000, memtotal-0x400000);
    
    //初始化
    init_gdtidt();
    init_pic();
    init_palette();
    init_pit();
    struct MOUSE_DEC mdec;
    init_keyboard(&fifo,256);
    enable_mouse(&fifo,512,&mdec);

    
    //初始化图册相关
    struct SHTCTL *shtctl = shtctl_init(memman, bInfo->vram, bInfo->scrnx, bInfo->scrny);
    
    struct SHEET *sht_back = sheet_alloc(shtctl);
    unsigned char *buf_back = buf_back = (unsigned char *)memman_alloc_4k(memman, bInfo->scrnx* bInfo->scrny);
    sheet_setbuf(sht_back, buf_back, bInfo->scrnx, bInfo->scrny, -1);
    init_screen8(buf_back, bInfo->scrnx, bInfo->scrny);
    sheet_updown(sht_back, 0);
    sheet_slide(sht_back, 0, 0);
    
    struct SHEET *sht_mouse = sheet_alloc(shtctl);
    unsigned char buf_mouse[256];
    sheet_setbuf(sht_mouse, buf_mouse,  16, 16, 99);
    sheet_updown(sht_mouse, 5);
    int mx = 160,my = 100;
    init_mouse_cursor8(buf_mouse,99);
    sheet_slide(sht_mouse, mx, my);
    
    struct SHEET *sht_win = sheet_alloc(shtctl);
    unsigned char *buf_win = (unsigned char *)memman_alloc_4k(memman, 160*52);
    sheet_setbuf(sht_win, buf_win, 144, 52, -1);
    make_window8(buf_win, 144, 52, "task_a",1);
    make_textbox8(sht_win, 10, 30, sht_win->bxsize-20, 16, COL8_WHITE);
    sheet_updown(sht_win, 1);
    sheet_slide(sht_win, 8, 56);
    
    //多任务
    struct TASK *task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1,0);
    
    /* sht_cons */
    struct SHEET *sht_cons = sheet_alloc(shtctl);
    unsigned char *buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 透明色なし */
    make_window8(buf_cons, 256, 165, "console", 0);
    make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
    struct TASK *task_cons = task_alloc();
    task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons->tss.eip = (int)console_task;
    task_cons->tss.es = 1 * 8;
    task_cons->tss.cs = 2 * 8;
    task_cons->tss.ss = 1 * 8;
    task_cons->tss.ds = 1 * 8;
    task_cons->tss.fs = 1 * 8;
    task_cons->tss.gs = 1 * 8;
    *((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
    *((int *) (task_cons->tss.esp + 8)) = (int) memtotal;
    task_run(task_cons, 2, 2); /* level=2, priority=2 */
    sheet_updown(sht_cons, 2);
    sheet_slide(sht_cons, 32, 200);
    
    
    
    //用于显示闪烁的光标
    struct TIMER *timer = timer_alloc();
    timer_init(timer, &fifo, 0);
    timer_settime(timer,50);
    
    
    
    //打开部分中断
    io_sti();   //这个地方产生了一个bug,调试了好久.....
    io_out8(PIC0_IMR, 0xf8);
    io_out8(PIC1_IMR, 0xef);
    
    
	//各种打印
    boxfill8(buf_back, bInfo->scrnx, COL8_RED, 0, 0, 310, 18);
	putfonts8_asc(buf_back, bInfo->scrnx,1,1,COL8_YELLOW,"HELLO YIOS");
	putfonts8_asc(buf_back, bInfo->scrnx,0,0,COL8_YELLOW,"HELLO YIOS");
    sheet_refresh(sht_back, 0, 0, 310, 18);
    // free 29304=632k(1m-4k(0开头的BIOS)-4k(后面的BIOS)-384k(0xa0000-0xaffff  显存用的地方 64k  后面的320我就不知道是干啥的了))+28m
    sprintf(s, "[total %dM, free %dK]",memtotal/1024/1024,memman_total(memman)/1024);
    putfonts8_asc_sht(sht_back, 0, 100, COL8_YELLOW, COL8_RED, s, 30);

    int key_to = 0,key_shift = 0,key_leds = (bInfo->leds >> 4) & 7;
    
    struct FIFO32 keycmd;
    int keycmd_wait = -1;
    int keycmd_buf[32];
    fifo32_init(&keycmd, 32, keycmd_buf, 0);
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);
    
    int cursor_x = 8,cursor_c = COL8_FFFFFF;
    int i;
	for(;;){
        if (fifo32_status(&keycmd) > 0 && keycmd_wait<0) {
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
        
        
        io_cli();
        if (fifo32_status(&fifo) ==0) {
            task_sleep(task_a);
            io_sti();
        }else{
            
            i = fifo32_get(&fifo);
            io_sti();
            
            if (256 <= i && i <= 511) {
                i-=256;
                
                sprintf(s, "jianpan %02X",i);
                putfonts8_asc_sht(sht_back, 0, 40, COL8_YELLOW, COL8_RED, s, 20);
                
                if (i < 0x80) {
                    
                    if (key_shift == 0) {
                        s[0] = keytable0[i];
                    }else{
                        s[0] = keytable1[i];
                    }
                }else{
                    s[0] = 0;
                }
                
                if ('A' <= s[0] && s[0] <= 'Z') {
                    if (((key_leds & 4)==0 && key_shift==0)||((key_leds & 4)!=0 && key_shift!=0)) {
                        s[0]+=0x20;
                    }
                }
                
                if (s[0]!=0) {
                    if (key_to==0) {
                        s[1] = 0;
                        putfonts8_asc_sht(sht_win, cursor_x, 30, COL8_RED, COL8_YELLOW, s, 1);
                        cursor_x+=8;
                    }else{
                        fifo32_put(&task_cons->fifo, s[0]+256);
                    }
                }
                
                if (i==0xe) { //退格
                    if (key_to==0) {
                        if (cursor_x > 8) {
                            putfonts8_asc_sht(sht_win, cursor_x, 30, COL8_WHITE, COL8_WHITE, " ", 1);
                            cursor_x -= 8;
                        }
                    }else{
                        fifo32_put(&task_cons->fifo, 8+256);
                    }
                    
                }else if(i==0xf){//tab
                    if (key_to==0) {
                        key_to = 1;
                        make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
                        cursor_c = -1;
                        boxfill8(buf_win, sht_win->bxsize, COL8_WHITE, cursor_x, 30, cursor_x+7, 45);
                        fifo32_put(&task_cons->fifo, 2);
                    }else{
                        key_to = 0;
                        make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
                        cursor_c = COL8_BLACK;
                        fifo32_put(&task_cons->fifo, 3);
                    }
                    
                    sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
                    sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
                }else if(i==0x2a){//左shift on
                    key_shift |= 1;
                }else if(i==0x36){
                    key_shift |= 2;
                }else if(i==0xaa){
                    key_shift &= ~1;
                }else if(i==0xb6){
                    key_shift &= ~2;
                }else if(i==0x3a){//caps
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }else if(i==0x45){//numlock
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }else if(i==0x46){//scrolllock
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }else if(i==0xfa){//向指示灯发送成功
                    keycmd_wait = -1;
                }else if(i==0xfe){//向指示灯发送失败
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }else if(i==0x1c){//回车键
                    if (key_to!=0) {
                        fifo32_put(&task_cons->fifo, 10+256);
                    }
                }
                
                if (cursor_c >=0) {
                    boxfill8(buf_win, sht_win->bxsize, cursor_c, cursor_x, 30, cursor_x+7, 45);
                }
                sheet_refresh( sht_win, cursor_x, 30, cursor_x+8, 46);
                
                
            }else if(512 <= i && i <= 767){
                i-=512;
                if (mouse_decode(&mdec, i) != 0) {
                    
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
            }else if(i<=1){
                if (i==0) {
                    timer_init(timer, &fifo, 1);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_BLACK;
                    }
                    
                }else{
                    timer_init(timer, &fifo, 0);
                    if (cursor_c >= 0) {
                        cursor_c = COL8_WHITE;
                    }
                }
                timer_settime(timer, 50);
                if (cursor_c>=0) {
                    boxfill8(buf_win, sht_win->bxsize, cursor_c, cursor_x, 30, cursor_x+7, 45);
                    sheet_refresh( sht_win, cursor_x, 30, cursor_x+8, 46);
                }
            }
        }
	}
    
    
}



void make_window8(unsigned char *buf, int xsize, int ysize, char *title,char act)
{
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
    boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
    
    make_wtitle8(buf,xsize,title,act);
    
}

void make_wtitle8(unsigned char *buf, int xsize,char *title, char act) {
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
    char c,tc,tbc;
    if (act !=0) {
        tc = COL8_FFFFFF;
        tbc = COL8_000084;
    }else{
        tc = COL8_C6C6C6;
        tbc = COL8_848484;
    }
    
    boxfill8(buf, xsize, tbc        , 3,         3,         xsize - 4, 20       );
    putfonts8_asc(buf, xsize, 24, 4, tc, title);
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

int cons_newline(int cursor_y, struct SHEET *sheet){
    int x,y;
    if (cursor_y < 28+112) {
        cursor_y+=16;
    }else{
        for (y = 28; y<28+112; y++) {
            for (x = 8; x < 8+240; x++) {
                sheet->buf[x+y*sheet->bxsize] = sheet->buf[x+(y+16)*sheet->bxsize];
            }
        }
        
        for (y = 28+112; y < 28+128; y++) {
            for (x = 8; x < 8+240; x++) {
                sheet->buf[x+y*sheet->bxsize] = COL8_BLACK;
            }
        }
        sheet_refresh(sheet, 8, 28, 8+240, 28+128);
    }
    return cursor_y;
}
