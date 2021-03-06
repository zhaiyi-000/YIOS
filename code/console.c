//
//  console.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/7.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"


void cmd_exit(struct CONSOLE *cons, int *fat);
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal);
void cmd_langmode(struct CONSOLE *cons, char *cmdline);

void console_task(struct SHEET *sheet, unsigned int memtotal) {
    struct TIMER *timer = NULL;
    struct TASK *task = task_now();
    struct FIFO32 *fifo = &task->fifo;

    int i;//, fifobuf[128];
    //fifo32_init(fifo, 128, fifobuf, task);
    char cmdline[30];
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    
    int *fat = (int *)memman_alloc_4k(memman, 4*2880);//共有2*18*80=2880个扇区
    //2880*1.5=4320个字节,9个扇区9*512=4608>4320
    file_readfat(fat, (unsigned char *)(ADR_DISKIMG+0x200));

    struct CONSOLE cons;
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;
    cons.timer = timer;
    task->cons = &cons;
    
    if (cons.sht!=0) {
        timer = timer_alloc();
        timer_init(timer, fifo, 0);
        timer_settime(timer, 50);
    }
    
    cons_putchar(&cons,'>',1);
    
    //文件handle
    struct FILEHANDLE fhandle[8];
    for (i = 0; i < 8; i++) {
        fhandle[i].buf = 0; //未使用标记
    }
    task->fhandle = fhandle;
    task->fat = fat;
    task->cmdline = cmdline;
    
    unsigned char *nihongo = (char *)*((int *)0xfe8);
    if (nihongo[4096]!=0xff) {
        task->langmode = 3;
    }else{
        task->langmode = 0;
    }
    task->langbyte1 = 0;

    for (;;) {
        io_cli();
        if (fifo32_status(fifo) == 0) {
            task_sleep(task);
            io_sti();
        } else {
            i = fifo32_get(fifo);
            io_sti();
            if (i <= 1 && cons.sht != 0) { /* カーソル用タイマ */
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
                if (cons.sht != 0) {
                    boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_BLACK, cons.cur_x, cons.cur_y, cons.cur_x+7, 43);
                }
                cons.cur_c = -1;
            }else if (i ==4) { //点击了关闭按钮
                cmd_exit(&cons, fat);
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
                    if (cons.sht==0) {
                        cmd_exit(&cons, fat);
                    }
                    cons_putchar(&cons, '>', 1);
                }else{
                    if (cons.cur_x < 240) {
                        cmdline[cons.cur_x/8-2] = i;
                        cons_putchar(&cons, i, 1);
                    }
                }
            }
            
            if (cons.sht!=0) {
                if (cons.cur_c >= 0) {
                    boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y+15);
                }
                sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y+16);
            }
        }
    }
}


void cons_newline(struct CONSOLE *cons){
    int x,y;
    struct SHEET *sheet = cons->sht;
    struct TASK *task = task_now();
    if (cons->cur_y < 28+112) {
        cons->cur_y +=16;
    }else{
        if (sheet!=0) {
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
    }
    cons->cur_x = 8;
    if (task->langmode==1 && task->langbyte1 !=0) {
        cons->cur_x+=8;
    }
}


void cons_putchar(struct CONSOLE *cons, int chr,char move) {
    char s[2];
    s[0] = chr;
    s[1] = 0;
    if (s[0]==0x9) {
        for (; ; ) {
            if (cons->sht!=0) {
                putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, " ", 1);
            }
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
        if (cons->sht!=0) {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 1);
        }
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

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    char *p,*q,name[18];
    int i;
    struct TASK *task = task_now();
    int segsiz,datsiz,esp,dathrb;
    struct SHTCTL *shtctl;
    struct SHEET *sht;
    
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
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        if (finfo->size >= 36 && strncmp(p+4, "Hari", 4)==0 && *p == 0) {
            segsiz = *((int *)(p));
            esp = *((int *)(p+0xc));
            datsiz = *((int *)(p+0x10));
            dathrb = *((int *)(p+0x14));
            q = (char *)memman_alloc_4k(memman, segsiz);
            task->ds_base = (int)q;
            
            set_segmdesc(task->ldt+0, finfo->size - 1, (int) p, AR_CODE32_ER+0x60);//p443
            set_segmdesc(task->ldt+1, segsiz - 1, (int) q, AR_DATA32_RW+0x60);
            
            for (i = 0; i < datsiz; i++) {
                q[esp+i] = p[dathrb+i];
            }
            
            start_app(0x1b, 0*8+4, esp, 1*8+4,&(task->tss.esp0));
            shtctl = (struct SHTCTL *)*((int *)0xfe4);
            for (i = 0; i < MAX_SHEETS; i++) {
                sht = &(shtctl->sheets0[i]);
                if ((sht->flags & 0x11)==0x11 && sht->task == task) {
                    sheet_free(sht);
                }
            }
            
            for (i = 0; i < 8; i++) {
                if (task->fhandle[i].buf != 0) {
                    memman_free_4k(memman, (int)task->fhandle[i].buf, task->fhandle[i].size);
                    task->fhandle[i].buf = 0;
                }
            }
            
            timer_cancelall(&task->fifo);
            memman_free_4k(memman, (int) q, segsiz);
            task->langbyte1 = 0;
        }else{
            cons_putstr0(cons,".hrb file format error.\n");
        }
        memman_free_4k(memman, (int) p, finfo->size);
        cons_newline(cons);
        return 1;
    }
    
    return 0;
}

void cmd_exit(struct CONSOLE *cons, int *fat){
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    struct TASK *task = task_now();
    struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0xfe4);
    struct FIFO32 *fifo = (struct FIFO32 *)*((int *)0xfec);
    if (cons->sht!=0) {
        timer_cancel(cons->timer);
    }
    memman_free_4k(memman, (int)fat, 4*2880);
    io_cli();
    if (cons->sht!=0) {
        fifo32_put(fifo, cons->sht-shtctl->sheets0+768);
    }else{
        fifo32_put(fifo, task-taskctl->task0+1024);
    }
    io_sti();
    for (; ; ) {
        task_sleep(task);
    }
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal) {
    if (strcmp(cmdline,"mem")==0 && cons->sht !=0) {
        cmd_mem(cons,memtotal);
    }else if (strcmp(cmdline,"cls")==0 && cons->sht !=0) {
        cmd_cls(cons);
    }else if (strcmp(cmdline,"dir")==0 && cons->sht !=0) {
        cmd_dir(cons);
    }else if (strcmp(cmdline, "exit")==0) {
        cmd_exit(cons, fat);
    }else if (strncmp(cmdline, "start ", 6)==0) {
        cmd_start(cons,cmdline,memtotal);
    }else if (strncmp(cmdline, "ncst ", 5)==0) {
        cmd_ncst(cons,cmdline,memtotal);
    }else if (strncmp(cmdline, "langmode ", 9)==0) {
        cmd_langmode(cons,cmdline);
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

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
    int i, x, y, len, dx, dy;

    dx = x1 - x0;
    dy = y1 - y0;
    x = x0 << 10;
    y = y0 << 10;
    if (dx < 0) {
        dx = - dx;
    }
    if (dy < 0) {
        dy = - dy;
    }
    if (dx >= dy) {
        len = dx + 1;
        if (x0 > x1) {
            dx = -1024;
        } else {
            dx =  1024;
        }
        if (y0 <= y1) {
            dy = ((y1 - y0 + 1) << 10) / len;
        } else {
            dy = ((y1 - y0 - 1) << 10) / len;
        }
    } else {
        len = dy + 1;
        if (y0 > y1) {
            dy = -1024;
        } else {
            dy =  1024;
        }
        if (x0 <= x1) {
            dx = ((x1 - x0 + 1) << 10) / len;
        } else {
            dx = ((x1 - x0 - 1) << 10) / len;
        }
    }

    for (i = 0; i < len; i++) {
        sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
        x += dx;
        y += dy;
    }

    return;
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
    struct TASK *task = task_now();
    int ds_base = task->ds_base;
    struct CONSOLE *cons = task->cons;
    struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0xfe4);
    struct FIFO32 *sys_fifo = (struct FIFO32 *)*((int *)0xfec);
    struct SHEET *sht;
    int *reg = &eax+1; //强行改先pushad的eax
    int i;
    struct FILEINFO *finfo;
    struct FILEHANDLE *fh;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    
    if (edx==1) {
        cons_putchar(cons, eax & 0xff, 1);
    }else if(edx ==2){
        cons_putstr0(cons, (char *)ebx+ds_base);
    }else if(edx ==3){
        cons_putstr1(cons, (char *)ebx,ecx+ds_base);
    }else if(edx ==4){
        return &(task->tss.esp0);
    }else if(edx ==5){ //openwin
        sht = sheet_alloc(shtctl);
        sht->task = task;
        sht->flags |= 0x10;
        sheet_setbuf(sht, (char *)ebx+ds_base, esi, edi, eax);
        make_window8((char *)ebx+ds_base, esi, edi, (char *)ecx+ds_base, 0);
        sheet_slide(sht, (shtctl->xsize-esi)/2, (shtctl->ysize-edi)/2);
        sheet_updown(sht, shtctl->top);
        reg[7] = (int)sht;
    }else if(edx ==6){
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp+ds_base);
        if ((ebx & 1)==0) {
            sheet_refresh(sht, esi, edi, esi+ecx*8, edi+16);
        }
    }else if(edx ==7){
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi,edi);
        if ((ebx & 1)==0) {
            sheet_refresh(sht, eax, ecx, esi+1, edi+1);
        }
    }else if(edx ==8){
        memman_init((struct MEMMAN *)(ebx+ds_base));
        ecx &= 0xfffffff0;
        memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
    }else if(edx ==9){
        ecx = (ecx+0x0f)&0xfffffff0;
        reg[7] = memman_alloc((struct MEMMAN *)(ebx+ds_base), ecx);
    }else if(edx ==10){
        ecx = (ecx+0xf)&0xfffffff0;
        memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
    }else if(edx ==11){
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        sht->buf[sht->bxsize*edi+esi]=eax;
        if ((ebx & 1)==0) {
            sheet_refresh(sht, esi, edi, esi+1, edi+1);
        }
    }else if(edx ==12){
        sht = (struct SHEET *)ebx;
        sheet_refresh(sht, eax, ecx, esi, edi);
    }else if(edx ==13){
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        hrb_api_linewin(sht,eax,ecx,esi,edi,ebp);
        if ((ebx & 1)==0) {
            if (eax > esi) {
                i = eax;
                eax = esi;
                esi = i;
            }
            if (ecx > edi) {
                i = ecx;
                ecx = edi;
                edi = i;
            }
            sheet_refresh(sht, eax, ecx, esi+1, edi+1);
        }
    }else if(edx ==14){
        sheet_free((struct SHEET *) ebx);
    }else if (edx == 15) {
        for (;;) {
            io_cli();
            if (fifo32_status(&task->fifo) == 0) {
                if (eax != 0) {
                    task_sleep(task);    /* FIFOが空なので寝て待つ */
                } else {
                    io_sti();
                    reg[7] = -1;
                    return 0;
                }
            }
            i = fifo32_get(&task->fifo);
            io_sti();
            if (i <= 1) { /* カーソル用タイマ */
                /* アプリ実行中はカーソルが出ないので、いつも次は表示用の1を注文しておく */
                timer_init(cons->timer, &task->fifo, 1);  //这个地方写0写1一个样
                timer_settime(cons->timer, 50);
            }
            if (i == 2) {    //2.3的作用，就是在这个api接管键盘的时候，让 cur_c 保持一致，起不到刷新图册的作用,没什么用吧 ，我可以注释掉。因为如果console_task响应 那么 i必为2  注:25.9这个产生了一个bug.前面这句话不对，因为可能是子窗口响应的
                cons->cur_c = COL8_FFFFFF;
            }
            if (i == 3) {    /* カーソルOFF */
                cons->cur_c = -1;
            }
            if (i==4) {
                timer_cancel(cons->timer);
                io_cli();
                fifo32_put(sys_fifo, cons->sht-shtctl->sheets0+2024);
                cons->sht = 0;
                io_sti();
            }
            if (256 <= i) { /* キーボードデータ（タスクA経由） */
                reg[7] = i - 256;
                return 0;
            }
        }
    }else if (edx == 16) {
        reg[7] = (int) timer_alloc();
        ((struct TIMER *) reg[7])->flags2 = 1;    /* 自動キャンセル有効 */
    } else if (edx == 17) {
        timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
    } else if (edx == 18) {
        timer_settime((struct TIMER *) ebx, eax);
    } else if (edx == 19) {
        timer_free((struct TIMER *) ebx);
    } else if (edx == 20) {
        if (eax == 0) {
            i = io_in8(0x61);
            io_out8(0x61, i & 0x0d);
        } else {
            i = 1193180000 / eax;
            io_out8(0x43, 0xb6);
            io_out8(0x42, i & 0xff);
            io_out8(0x42, i >> 8);
            i = io_in8(0x61);
            io_out8(0x61, (i | 0x03) & 0x0f);
        }
    } else if (edx == 21) {
        for (i = 0; i < 8; i++) {
            if (task->fhandle[i].buf == 0) {
                break;
            }
        }
        fh = &task->fhandle[i];
        reg[7] = 0;
        if (i < 8) {
            finfo = file_search((char *) ebx + ds_base,
                    (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
            if (finfo != 0) {
                reg[7] = (int) fh;
                fh->buf = (char *) memman_alloc_4k(memman, finfo->size);
                fh->size = finfo->size;
                fh->pos = 0;
                file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, (char *) (ADR_DISKIMG + 0x003e00));
            }
        }
    } else if (edx == 22) {
        fh = (struct FILEHANDLE *) eax;
        memman_free_4k(memman, (int) fh->buf, fh->size);
        fh->buf = 0;
    } else if (edx == 23) {
        fh = (struct FILEHANDLE *) eax;
        if (ecx == 0) {
            fh->pos = ebx;
        } else if (ecx == 1) {
            fh->pos += ebx;
        } else if (ecx == 2) {
            fh->pos = fh->size + ebx;
        }
        if (fh->pos < 0) {
            fh->pos = 0;
        }
        if (fh->pos > fh->size) {
            fh->pos = fh->size;
        }
    } else if (edx == 24) {
        fh = (struct FILEHANDLE *) eax;
        if (ecx == 0) {
            reg[7] = fh->size;
        } else if (ecx == 1) {
            reg[7] = fh->pos;
        } else if (ecx == 2) {
            reg[7] = fh->pos - fh->size;
        }
    } else if (edx == 25) {
        fh = (struct FILEHANDLE *) eax;
        for (i = 0; i < ecx; i++) {
            if (fh->pos == fh->size) {
                break;
            }
            *((char *) ebx + ds_base + i) = fh->buf[fh->pos];
            fh->pos++;
        }
        reg[7] = i;
    } else if (edx == 26) {
        i = 0;
        for (;;) {
            *((char *) ebx + ds_base + i) =  task->cmdline[i];
            if (task->cmdline[i] == 0) {
                break;
            }
            if (i >= ecx) {
                break;
            }
            i++;
        }
        reg[7] = i;
    }
//    else if (edx == 27) {
//        reg[7] = task->langmode;
//    }
    
    return 0;
}


int *inthandler0d(int *esp){
    struct TASK *task = task_now();
    struct CONSOLE *cons = task->cons;
    char s[30];
    
    cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}

int *inthandler0c(int *esp){
    struct TASK *task = task_now();
    struct CONSOLE *cons = task->cons;
    char s[30];
    
    cons_putstr0(cons, "\nINT 0C :\n General Protected Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}


void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal)
{
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
    struct SHEET *sht = open_console(shtctl, memtotal);
    struct FIFO32 *fifo = &sht->task->fifo;
    int i;
    sheet_slide(sht, 32, 4);
    sheet_updown(sht, shtctl->top);
    /* コマンドラインに入力された文字列を、一文字ずつ新しいコンソールに入力 */
    for (i = 6; cmdline[i] != 0; i++) {
        fifo32_put(fifo, cmdline[i] + 256);
    }
    fifo32_put(fifo, 10 + 256);    /* Enter */
    cons_newline(cons);
    return;
}

void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal)
{
    struct TASK *task = open_constask(0, memtotal);
    struct FIFO32 *fifo = &task->fifo;
    int i;
    /* コマンドラインに入力された文字列を、一文字ずつ新しいコンソールに入力 */
    for (i = 5; cmdline[i] != 0; i++) {
        fifo32_put(fifo, cmdline[i] + 256);
    }
    fifo32_put(fifo, 10 + 256);    /* Enter */
    cons_newline(cons);
    return;
}


void cmd_langmode(struct CONSOLE *cons, char *cmdline)
{
    struct TASK *task = task_now();
    unsigned char mode = cmdline[9] - '0';
    if (mode <= 3) {
        task->langmode = mode;
    } else {
        cons_putstr0(cons, "mode number error.\n");
    }
    cons_newline(cons);
    return;
}
