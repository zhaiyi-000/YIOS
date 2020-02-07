//
//  console.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/7.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

void console_task(struct SHEET *sheet, unsigned int memtotal) {
    int x,y;
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct FIFO32 *fifo = &task->fifo;
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG+0x2600);
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

    int i, fifobuf[128], cursor_x = 16,cursor_y = 28, cursor_c = -1;
    fifo32_init(fifo, 128, fifobuf, task);
    char s[100],cmdline[30],*p;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    
    int *fat = (int *)memman_alloc_4k(memman, 4*2880);//共有2*18*80=2880个扇区
    //2880*1.5=4320个字节,9个扇区9*512=4608>4320
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG+0x200));

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
                    
                    if (strcmp(cmdline,"mem")==0) {
                        sprintf(s, "total %dMB", memtotal/1024/1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        
                        sprintf(s, "free %dKB", memman_total(memman)/1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }else if (strcmp(cmdline,"cls")==0) {
                        for (y = 28; y < 28+128; y++) {
                            for (x = 8; x < 8+240; x++) {
                                sheet->buf[x+y*sheet->bxsize] = COL8_BLACK;
                            }
                        }
                        sheet_refresh(sheet, 8, 28, 8+240, 28+128);
                        cursor_y = 28;
                    }else if (strcmp(cmdline,"dir")==0) {
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
                    }else if (strncmp(cmdline, "type ", 5)==0) {
                        for (y = 0; y < 11; y++) {
                            s[y] = ' ';
                        }
                        y =0;
                        for (x = 5; y < 11 && cmdline[x]!=0; x++) {
                            if (cmdline[x]=='.' && y <= 8) {
                                y = 8;
                            }else{
                                s[y] = cmdline[x];
                                if ('a'<=s[y] && s[y]<='z') {
                                    s[y]-=0x20;
                                }
                                y++;
                            }
                        }
                        
                        for (x = 0; x < 224; ) {
                            if (finfo[x].name[0] == 0) {
                                break;
                            }
                            if ((finfo[x].type & 0x18)==0) {
                                for (y = 0; y < 11; y++) {
                                    if (finfo[x].name[y]!=s[y]) {
                                        goto type_next_file;
                                    }
                                }
                                break;   //找到文件
                            }
                            
                        type_next_file:
                            x++;
                        }
                        
                        if (x < 224 && finfo[x].name[0]!=0) {
                            //找到文件
                            p = (char *)memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *)(ADR_DISKIMG+0x3e00));
                            
                            cursor_x = 8;
                            
                            for (y = 0; y < finfo[x].size; y++) {
                                s[0] = p[y];
                                s[1] = 0;
                                if (s[0]==0x9) {
                                    for (; ; ) {
                                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, " ", 1);
                                        cursor_x+=8;
                                        if (cursor_x==8+240) {
                                            cursor_x = 8;
                                            cursor_y = cons_newline(cursor_y, sheet);
                                        }
//                                        if ((cursor_x-8)%32==0) {
                                        if (((cursor_x-8)&0x1f)==0) { //这两种都可以,下面这种应该要快些
                                            break;
                                        }
                                    }
                                }else if(s[0]==0xa){
                                    cursor_x = 8;
                                    cursor_y = cons_newline(cursor_y, sheet);
                                }else if(s[0]==0xd){
                                }else{
                                    putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, s, 1);
                                    cursor_x+=8;
                                    if (cursor_x==8+240) {
                                        cursor_x = 8;
                                        cursor_y = cons_newline(cursor_y, sheet);
                                    }
                                }
                            }
                            
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        }else{
                            putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, "FILE NOT FOUND", 15);
                            cursor_y = cons_newline(cursor_y, sheet);
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                        
                    }else if (strcmp(cmdline, "hlt")==0) {
                        for (y = 0; y < 11; y++) {
                            s[y] = ' ';
                        }
                        s[0] = 'H';
                        s[1] = 'L';
                        s[2] = 'T';
                        s[8] = 'H';
                        s[9] = 'R';
                        s[10] = 'B';
                        
                        for (x = 0; x < 224; ) {
                            if (finfo[x].name[0] == 0) {
                                break;
                            }
                            if ((finfo[x].type & 0x18)==0) {
                                for (y = 0; y < 11; y++) {
                                    if (finfo[x].name[y]!=s[y]) {
                                        goto hlt_next_file;
                                    }
                                }
                                break;   //找到文件
                            }
                            
                        hlt_next_file:
                            x++;
                        }
                        
                        if (x < 224 && finfo[x].name[0]!=0) {
                            //找到文件
                            p = (char *)memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *)(ADR_DISKIMG+0x3e00));
                            
                            set_segmdesc(gdt+1003, finfo[x].size-1, (int)p, AR_CODE32_ER);
                            farjmp(0, 1003*8);
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        }else{
                            putfonts8_asc_sht(sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, "FILE NOT FOUND", 15);
                            cursor_y = cons_newline(cursor_y, sheet);
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

