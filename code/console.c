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
    
    sprintf(s, "total %dMB", memtotal/1024/1024);
    putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 30);
    cons_newline(cons);
    
    sprintf(s, "free %dKB", memman_total(memman)/1024);
    putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 30);
    cons_newline(cons);
    cons_newline(cons);
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
                sprintf(s, "filename.exe    %7d",finfo[x].size);
                for (y = 0; y < 8; y++) {
                    s[y] = finfo[x].name[y];
                }
                for (y = 0; y < 3; y++) {
                    s[9+y] = finfo[x].ext[y];
                }
                putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 30);
                cons_newline(cons);
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
    int i;
    if (finfo != 0) {
        /* ファイルが見つかった場合 */
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        for (i = 0; i < finfo->size; i++) {
            cons_putchar(cons, p[i], 1);
        }
        memman_free_4k(memman, (int) p, finfo->size);
    } else {
        /* ファイルが見つからなかった場合 */
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
        cons_newline(cons);
    }
    cons_newline(cons);
    return;
}

void cmd_hlt(struct CONSOLE *cons, int *fat)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo = file_search("HLT.HRB", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    char *p;
    if (finfo != 0) {
        /* ファイルが見つかった場合 */
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER);
        farcall(0, 1003 * 8);
        memman_free_4k(memman, (int) p, finfo->size);
    } else {
        /* ファイルが見つからなかった場合 */
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
        cons_newline(cons);
    }
    cons_newline(cons);
    return;
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
    }else if (strcmp(cmdline, "hlt")==0) {
        cmd_hlt(cons,fat);
    }else if(cmdline[0]!=0){
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_WHITE, COL8_BLACK, "Bad command.", 30);
        cons_newline(cons);
        cons_newline(cons);
    }
}
