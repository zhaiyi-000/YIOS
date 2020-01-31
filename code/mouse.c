//
//  mouse.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/31.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"



void enable_mouse(struct MOUSE_DEC *mdec) {
    wait_KBC_sendready();
    io_out8(0x64, 0xd4);
    wait_KBC_sendready();
    io_out8(0x60, 0xf4);  // 0x64是控制和状态端口,0x60是数据端口
    mdec->phase = 0;
}


int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data){
    if (mdec->phase==0) {
        if (data==0xfa) {
            mdec->phase=1;
        }
        return 0;
    }else if(mdec->phase==1){
        if ((data & 0xc8)==0x08) {
            mdec->phase=2;
            mdec->buf[0] = data;
        }
        return 0;
    }else if(mdec->phase==2){
        mdec->phase=3;
        mdec->buf[1] = data;
        return 0;
    }else if(mdec->phase==3){
        mdec->phase=1;
        mdec->buf[2] = data;
        
        mdec->btn = mdec->buf[0] & 0x7;
        mdec->x = (char)mdec->buf[1];
        mdec->y = (char)mdec->buf[2];
        mdec->y = -mdec->y;
        return 1;
    }else{
        return -1;
    }
}

