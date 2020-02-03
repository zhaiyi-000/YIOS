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
    io_out8(PIC1_ICW2, 0x28); //irq8-15由int28-2f接受
    io_out8(PIC1_ICW3, 2);  //pic1由irq2接受
    io_out8(PIC1_ICW4, 0x1);
    
    io_out8(PIC0_IMR, 0xfb);  //除了2号以外全部屏幕 2号是从pic
    io_out8(PIC1_IMR, 0xff);
    
}

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void inthandler21(int esp){  //源代码写的是int *,先不管
    char data;
    
    io_out8(PIC0_OCW2, 0x61);
    data = io_in8(0x60);
    fifo8_put(&keyfifo, data);
}

void inthandler2c(int esp){  //源代码写的是int *,先不管
    char data;
    
    io_out8(PIC1_OCW2, 0x64);
    io_out8(PIC0_OCW2, 0x62);
    data = io_in8(0x60);
    fifo8_put(&mousefifo, data);
}

void inthandler27(int *esp)
/* PIC0からの不完全割り込み対策 */
/* Athlon64X2機などではチップセットの都合によりPICの初期化時にこの割り込みが1度だけおこる */
/* この割り込み処理関数は、その割り込みに対して何もしないでやり過ごす */
/* なぜ何もしなくていいの？
    →  この割り込みはPIC初期化時の電気的なノイズによって発生したものなので、
        まじめに何か処理してやる必要がない。                                    */
{
    io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
    return;
}
