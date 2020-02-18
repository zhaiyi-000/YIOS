#include "bootpack.h"

void yiPrintf(char *chs){
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(bInfo->vram, bInfo->scrnx, COL8_RED, 0, 120, 310, 136);
    putfonts8_asc(bInfo->vram, bInfo->scrnx, 0, 120, COL8_YELLOW, chs);
}

void keywin_on(struct SHEET *key_win);
void keywin_off(struct SHEET *key_win);
void close_constask(struct TASK *task);
void close_console(struct SHEET *sht);

void HariMain(){
    
    //数据
    static char keytable0[0x80] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x8,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0xa,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
    };
    static char keytable1[0x80] = {
        0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x8,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0xa,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    
    int j,x,y,mmx = -1,mmy = -1;
    struct SHEET *sht = 0,*key_win; //点击窗口调整图册用
    int i;
    
    struct BOOTINFO *bInfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[100];
    //缓冲区
    struct FIFO32 fifo;
    *((int *)0xfec) = (int)&fifo;
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
    *((int *)0xfe4) = (int)shtctl;
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
    
    //多任务
    struct TASK *task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1,0);
    
    /* sht_cons */
    key_win = open_console(shtctl, memtotal);
    keywin_on(key_win);
    sheet_updown(key_win, 1);
    sheet_slide(key_win, 8, 200);
    
    
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

    int key_shift = 0,key_leds = (bInfo->leds >> 4) & 7;
    
    struct FIFO32 keycmd;
    int keycmd_wait = -1;
    int keycmd_buf[32];
    fifo32_init(&keycmd, 32, keycmd_buf, 0);
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);
    
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
            
            if (key_win !=0 && key_win->flags == 0) {
                if (shtctl->top==1) {
                    key_win = 0;
                }else{
                    key_win = shtctl->sheets[shtctl->top-1];
                    keywin_on(key_win);
                }
            }
            
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
                
                if (s[0]!=0 && key_win !=0) {
                    fifo32_put(&key_win->task->fifo, s[0]+256);
                }
                
                if(i==0xf && key_win !=0){//tab
                    keywin_off(key_win);
                    j = key_win->height-1;
                    if (j==0) {
                        j = shtctl->top-1;
                    }
                    key_win = shtctl->sheets[j];
                    keywin_on(key_win);
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
                }else if(i==0x3b && key_shift != 0 && key_win !=0) {//shift+f1 强制结束
                    struct TASK *task = key_win->task;
                    if (task!=0 && task->tss.ss0 !=0) {
                        cons_putstr0(task->cons, "\nBreak(key) :\n");
                        io_cli();
                        task->tss.eax = (int)&(task->tss.esp0);
                        task->tss.eip = (int)asm_end_app;
                        io_sti();
                    }
                }else if(i==0x44 && shtctl->top >2){//f10 // 调整图册
                    sheet_updown(shtctl->sheets[1], shtctl->top-1);
                }else if(i==0x58 && key_shift!=0) {//新建console
                    if ( key_win !=0) {
                        keywin_off(key_win);
                    }
                    key_win = open_console(shtctl, memtotal);
                    sheet_slide(key_win, 32, 400);
                    sheet_updown(key_win, shtctl->top);
                    keywin_on(key_win);
                }
                
                
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
//                        /* 左ボタンを押していたら、sht_winを動かす */
//                        sheet_slide(sht_win, mx - 80, my - 8);
                        if (mmx<0) {
                            for (j = shtctl->top-1; j > 0; j--) {
                                sht = shtctl->sheets[j];
                                x = mx-sht->vx0;
                                y = my-sht->vy0;
                                if (0<=x && x<sht->bxsize && 0<=y && y < sht->bysize) {
                                    if (sht->buf[y*sht->bxsize+x]!=sht->col_inv) {
                                        sheet_updown(sht, shtctl->top-1);
                                        if (sht!=key_win) {
                                            keywin_off(key_win);
                                            key_win = sht;
                                            keywin_on(key_win);
                                        }
                                        
                                        if (3<=x && x < sht->bxsize && 3 <=y && y<21) {
                                            mmx = mx;
                                            mmy = my;
                                        }
                                        if (sht->bxsize-21<=x&&x<sht->bxsize-5&&
                                            5<=y && y<19) {
                                            //点击了x按钮
                                            if ((sht->flags&0x10)!=0) {
                                                struct TASK *task = sht->task;
                                                cons_putstr0(task->cons, "\nBreak(mouse):\n");
                                                io_cli();
                                                task->tss.eax = (int)&(task->tss.esp0);
                                                task->tss.eip = (int)asm_end_app;
                                                io_sti();
                                            }else{
                                                struct TASK *task = sht->task;
                                                io_cli();
                                                fifo32_put(&task->fifo, 4);
                                                io_sti();
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }else{
                            x = mx-mmx;
                            y = my-mmy;
                            sheet_slide(sht, sht->vx0+x, sht->vy0+y);
                            mmx = mx;
                            mmy = my;
                        }
                    }else{
                        mmx = -1;
                    }
                }
            }else if(768 <= i && i <= 1023){
                close_console(shtctl->sheets0+(i-768));
            }else if(1024 <= i && i <= 2023){
                close_constask(taskctl->task0+(i-1024));
            }
        }
	}
    
    
}

void keywin_off(struct SHEET *key_win)
{
    change_wtitle8(key_win, 0);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 3); /* コンソールのカーソルOFF */
    }
}

void keywin_on(struct SHEET *key_win)
{
    change_wtitle8(key_win, 1);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 2); /* コンソールのカーソルON */
    }
}


struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHEET *sht = sheet_alloc(shtctl);
    unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht, buf, 256, 165, -1); /* 透明色なし */
    make_window8(buf, 256, 165, "console", 0);
    make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
    sht->task = open_constask(sht, memtotal);
    sht->flags |= 0x20;
    return sht;
}

void close_constask(struct TASK *task)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    task_sleep(task);
    memman_free_4k(memman, task->cons_stack, 64 * 1024);
    memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
    task->flags = 0; /* task_free(task); の代わり */
    return;
}

void close_console(struct SHEET *sht)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = sht->task;
    memman_free_4k(memman, (int) sht->buf, 256 * 165);
    sheet_free(sht);
    close_constask(task);
    return;
}


struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = task_alloc();
    int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
    task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
    task->tss.esp = task->cons_stack + 64 * 1024 - 12;
    task->tss.eip = (int) &console_task;
    task->tss.es = 1 * 8;
    task->tss.cs = 2 * 8;
    task->tss.ss = 1 * 8;
    task->tss.ds = 1 * 8;
    task->tss.fs = 1 * 8;
    task->tss.gs = 1 * 8;
    *((int *) (task->tss.esp + 4)) = (int) sht;
    *((int *) (task->tss.esp + 8)) = memtotal;
    task_run(task, 2, 2); /* level=2, priority=2 */
    fifo32_init(&task->fifo, 128, cons_fifo, task);
    return task;
}
