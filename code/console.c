//
//  console.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/7.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

struct CONSOLE {
    struct SHEET *sht;
    int cur_x, cur_y, cur_c;
};

void cons_newline(struct CONSOLE *cons);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cons_putchar(struct CONSOLE *cons, int chr,char move);
void cons_putstr0(struct CONSOLE *cons, char *s);
void cons_putstr1(struct CONSOLE *cons, char *s,int l);


void console_task(struct SHEET *sheet, unsigned int memtotal) {
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct FIFO32 *fifo = &task->fifo;

    int i, fifobuf[128];
    fifo32_init(fifo, 128, fifobuf, task);
    char cmdline[30];
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    
    int *fat = (int *)memman_alloc_4k(memman, 4*2880);//共有2*18*80=2880个扇区
    //2880*1.5=4320个字节,9个扇区9*512=4608>4320
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG+0x200));

    timer = timer_alloc();
    timer_init(timer, fifo, 1);
    timer_settime(timer, 50);
    
    struct CONSOLE cons;
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;
    *((int *)0xfec) = (int)&cons;
    
    cons_putchar(&cons,'>',1);

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
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_FFFFFF;
                    }
                } else {
                    timer_init(timer, fifo, 1); /* 次は1を */
                    if (cons.cur_c >= 0) {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }else if (i == 2) { //显示光标
                cons.cur_c = COL8_WHITE;
            }else if (i ==3) { //不显示光标
                boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, cons.cur_x, cons.cur_y, cons.cur_x+7, 43);
                cons.cur_c = -1;
            }else if (256 <= i && i <=511) {
                i-=256;
                if (i==8) {
                    if (cons.cur_x > 16) {//退格键
                        cons_putchar(&cons, ' ', 0);
                        cons.cur_x -= 8;
                    }
                }else if (i==10){//回车
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x/8-2] = 0;
                    cons_newline(&cons);
                    cons_runcmd(cmdline, &cons, fat, memtotal);
                    cons_putchar(&cons, '>', 1);
                }else{
                    if (cons.cur_x < 240) {
                        cmdline[cons.cur_x/8-2] = i;
                        cons_putchar(&cons, i, 1);
                    }
                }
            }
            
            if (cons.cur_c >= 0) {
                boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y+15);
            }
            sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y+16);
        }
    }
}


void cons_newline(struct CONSOLE *cons){
    int x,y;
    struct SHEET *sheet = cons->sht;
    if (cons->cur_y < 28+112) {
        cons->cur_y +=16;
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
    cons->cur_x = 8;
}


void cons_putchar(struct CONSOLE *cons, int chr,char move) {
    char s[2];
    s[0] = chr;
    s[1] = 0;
    if (s[0]==0x9) {
        for (; ; ) {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, " ", 1);
            cons->cur_x+=8;
            if (cons->cur_x==8+240) {
                cons_newline(cons);
            }
            //                                        if ((cursor_x-8)%32==0) {
            if (((cons->cur_x-8)&0x1f)==0) { //这两种都可以,下面这种应该要快些
                break;
            }
        }
    }else if(s[0]==0xa){
        cons_newline(cons);
    }else if(s[0]==0xd){
    }else{
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 1);
        if (move != 0) {
            cons->cur_x +=8;
            if (cons->cur_x==8+240) {
                cons_newline(cons);
            }
        }
    }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal){
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    char s[30];
    
    sprintf(s, "total %dMB\nfree %dKB\n\n", memtotal/1024/1024,memman_total(memman)/1024);
    cons_putstr0(cons, s);
}

void cmd_cls(struct CONSOLE *cons){
    int x,y;
    struct SHEET *sheet = cons->sht;
    for (y = 28; y < 28+128; y++) {
        for (x = 8; x < 8+240; x++) {
            sheet->buf[x+y*sheet->bxsize] = COL8_BLACK;
        }
    }
    sheet_refresh(sheet, 8, 28, 8+240, 28+128);
    cons->cur_y = 28;
}

void cmd_dir(struct CONSOLE *cons){
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG+0x2600);
    int x,y;
    char s[30];
    
    for (x = 0; x < 224; x++) {
        if (finfo[x].name[0] == 0x0) {
            break;
        }
        if (finfo[x].name[0]!=0xe5) { //0xe5代表文件删除了
            if ((finfo[x].type & 0x18)==0) { //非目录,非非文件信息
                sprintf(s, "filename.exe    %7d\n",finfo[x].size);
                for (y = 0; y < 8; y++) {
                    s[y] = finfo[x].name[y];
                }
                for (y = 0; y < 3; y++) {
                    s[9+y] = finfo[x].ext[y];
                }
                cons_putstr0(cons, s);
            }
        }
    }
    cons_newline(cons);
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    char *p;
    if (finfo != 0) {
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        cons_putstr1(cons, p, finfo->size);
        memman_free_4k(memman, (int) p, finfo->size);
    } else {
        cons_putstr0(cons, "File not found.\n");
    }
    cons_newline(cons);
    return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    char *p,*q,name[18];
    int i;
    struct TASK *task = task_now();
    
    for (i = 0; i < 13; i++) {
        if (cmdline[i]<=' ') {  //小于空格的基本都是不可显示字符
            break;
        }
        name[i] = cmdline[i];
    }
    name[i] = 0;
    
    finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    if (finfo==0 && name[i-1]!='.') {
        name[i+0] = '.';
        name[i+1] = 'H';
        name[i+2] = 'R';
        name[i+3] = 'B';
        name[i+4] = 0;
        finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    }
    
    if (finfo != 0) {
        //找到文件的情况
        p = (char *) memman_alloc_4k(memman, finfo->size);
        q = (char *)memman_alloc_4k(memman, 64*1024);
        *((int *) 0xfe8) = (int)p;
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER+0x60);//p443
        set_segmdesc(gdt + 1004, 64*1024 - 1, (int) q, AR_DATA32_RW+0x60);
        
        if (finfo->size>=8 && strncmp(p+4, "Hari", 4)==0) { //这里的意思是:如果是c语言程序,需要call 1b,完了再retf p430
            p[0] = 0xe8;
            p[1] = 0x16;
            p[2] = 0x00;
            p[3] = 0x00;
            p[4] = 0x00;
            p[5] = 0xcb;
        }
        
        
        start_app(0, 1003*8, 64*1024, 1004*8,&(task->tss.esp0));
        memman_free_4k(memman, (int) p, finfo->size);
        memman_free_4k(memman, (int) q, 64*1024);
        cons_newline(cons);
        return 1;
    }
    
    return 0;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal) {
    if (strcmp(cmdline,"mem")==0) {
        cmd_mem(cons,memtotal);
    }else if (strcmp(cmdline,"cls")==0) {
        cmd_cls(cons);
    }else if (strcmp(cmdline,"dir")==0) {
        cmd_dir(cons);
    }else if (strncmp(cmdline, "type ", 5)==0) {
        cmd_type(cons,fat,cmdline);
    }else if(cmdline[0]!=0){
        if (cmd_app(cons, fat, cmdline)==0) {
            cons_putstr0(cons,"Bad command.\n\n");
        }
    }
}


void cons_putstr0(struct CONSOLE *cons, char *s){
    for (; *s != 0; s++) {
        cons_putchar(cons, *s, 1);
    }
}
void cons_putstr1(struct CONSOLE *cons, char *s,int l){
    int i;
    for (i = 0; i < l; i++) {
        cons_putchar(cons, s[i], 1);
    }
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
    int cs_base = *((int *)0xfe8);
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
    struct TASK *task = task_now();
    
    if (edx==1) {
        cons_putchar(cons, eax & 0xff, 1);
    }else if(edx ==2){
        cons_putstr0(cons, (char *)ebx+cs_base);
    }else if(edx ==3){
        cons_putstr1(cons, (char *)ebx,ecx+cs_base);
    }else if(edx ==4){
        return &(task->tss.esp0);
    }
    return 0;
}


int *inthandler0d(int *esp){
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
    struct TASK *task = task_now();
    char s[30];
    
    cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}

int *inthandler0c(int *esp){
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
    struct TASK *task = task_now();
    char s[30];
    
    cons_putstr0(cons, "\nINT 0C :\n General Protected Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}
