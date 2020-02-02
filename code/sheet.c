//
//  sheet.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/2.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"



struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    int i;
    struct SHTCTL *ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl==0) {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;  //一个sheet也没有
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0;  //标记为未使用
    }
    
err:
    return ctl;
}



struct SHEET *sheet_alloc(struct SHTCTL *ctl){
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags == 0) {
            sht = &ctl->sheets0[i];
            sht->flags = 1;
            sht->height = -1;
            return sht;
        }
    }
    
    return 0; //未申请成功
}


void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize,int col_inv){
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height) {
    int i,old = sht->height;
    if (height > ctl->top+1) {
        height = ctl->top+1;
    }
    if (height < -1) {
        height = -1;
    }
    sht->height = height;
    
    if (height < old) {
        if (height >= 0) {
            for (i = old; i > height; i--) {
                ctl->sheets[i] = ctl->sheets[i-1];
                ctl->sheets[i]->height = i;
            }
            ctl->sheets[height] = sht;
        }else{
            if (ctl->top>old) {
                for (i = old; i < ctl->top; i++) {
                    ctl->sheets[i] = ctl->sheets[i+1];
                    ctl->sheets[i]->height = i;
                }
            }
            ctl->top--;
        }
        
    } else if (height > old){
        if (old >= 0) {
            for (i = old; i < height; i++) {
                ctl->sheets[i] = ctl->sheets[i+1];
                ctl->sheets[i]->height = i;
            }
            ctl->sheets[height] = sht;
        }else{
            ctl->top++;
            for (i = ctl->top; i > height; i--) {
                ctl->sheets[i] = ctl->sheets[i-1];
                ctl->sheets[i]->height = i;
            }
            ctl->sheets[height] = sht;
        }
    }
    
    sheet_refresh(ctl);
    
}


void sheet_refresh(struct SHTCTL *ctl){
    int i,bx,by,vx,vy;
    unsigned char *buf,c,*vram = ctl->vram;
    struct SHEET *sht;
    
    for (i = 0; i <= ctl->top; i++) {
        sht = ctl->sheets[i];
        buf = sht->buf;
        
        for (by = 0; by < sht->bysize; by++) {
            vy = sht->vy0+by;
            
            for (bx = 0; bx < sht->bxsize; bx++) {
                vx = sht->vx0+bx;
                
                c = buf[by*sht->bxsize+bx];
                if (c != sht->col_inv) {
                    vram[vy*ctl->xsize+vx] = c;
                }
                
            }
        }
    }
}


void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0,int vy0){
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        sheet_refresh(ctl);
    }
}


void sheet_free(struct SHTCTL *ctl, struct SHEET *sht){
    if (sht->height >= 0) {
        sheet_updown(ctl, sht, -1);
    }
    sht->flags = 0;
}

