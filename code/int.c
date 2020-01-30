//
//  int.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/30.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"


void init_pic(void) {
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);
    
    io_out8(PIC0_ICW1, 0x11);
    io_out8(PIC0_ICW2, 0x20);  //irq0-7由int20-27接受
    io_out8(PIC0_ICW3, 1<<2);
    io_out8(PIC0_ICW4, 0x1);
    
    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW1, 0x28); //irq8-15由int28-2f接受
    io_out8(PIC1_ICW1, 2);  //pic1由irq2接受
    io_out8(PIC1_ICW1, 0x1);
    
    io_out8(PIC0_IMR, 0xfb);
    io_out8(PIC1_IMR, 0xff);
}
