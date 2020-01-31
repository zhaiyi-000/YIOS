//
//  keyboard.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/31.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"



void wait_KBC_sendready(void) {
    for (; ; ) {
        if ((io_in8(0x64)&0x2)==0) {
            break;
        }
    }
}

        
void init_keyboard(void){
    wait_KBC_sendready();
    io_out8(0x64, 0x60);
    wait_KBC_sendready();
    io_out8(0x60, 0x47);
}
