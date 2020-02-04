//
//  keyboard.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/31.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"


struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void) {
    for (; ; ) {
        if ((io_in8(0x64)&0x2)==0) {
            break;
        }
    }
}

        
void init_keyboard(struct FIFO32 *fifo, int data0){
    
    keyfifo = fifo;
    keydata0 = data0;
    
    wait_KBC_sendready();
    io_out8(0x64, 0x60);
    wait_KBC_sendready();
    io_out8(0x60, 0x47);
}

void inthandler21(int *esp){  //源代码写的是int *,先不管 21是键盘
    int data;
    
    io_out8(PIC0_OCW2, 0x61);
    data = io_in8(0x60);
    fifo32_put(keyfifo, data+keydata0);
}
