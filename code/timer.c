//
//  timer.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/3.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"


struct TIMECTL timerctl;

void init_pit(void) {
    io_out8(0x43, 0x34);
    io_out8(0x40, 0x9c);
    io_out8(0x40, 0x2e);
    
    timerctl.count = 0;
}


void inthandler20(int esp) {
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
}


