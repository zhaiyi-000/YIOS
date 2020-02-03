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
    int i;
    io_out8(0x43, 0x34);
    io_out8(0x40, 0x9c);
    io_out8(0x40, 0x2e);
    
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timer[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void){
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == 0) {
            timerctl.timer[i].flags = 1;
            return timerctl.timer+i;
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer){
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data) {
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout){
    timer->timeout = timeout;
    timer->flags = 2;
}


void inthandler20(int esp) {
    int i;
    struct TIMER *timer;
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    for (i = 0; i < MAX_TIMER; i++) {
        timer = &timerctl.timer[i];
        if (timer->flags == 2) {
            timer->timeout--;
            if (timer->timeout==0) {
                timer->flags = 1;
                fifo8_put(timer->fifo, timer->data);
            }
        }
    }
    
}

