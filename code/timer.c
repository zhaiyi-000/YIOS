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
    timerctl.next = 0xffffffff;
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void){
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers0[i].flags == 0) {
            timerctl.timers0[i].flags = 1;
            return timerctl.timers0+i;
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
    int e,i,j;
    timer->timeout = timeout + timerctl.count;
    timer->flags = 2;
    
    e = io_load_eflags();
    io_cli();
    
    for (i = 0; i < timerctl.using; i++) {
        if (timerctl.timers[i]->timeout >= timer->timeout) {
            break;
        }
    }
    for (j = timerctl.using; j > i; j--) {
        timerctl.timers[j] = timerctl.timers[j-1];
    }
    timerctl.using++;
    timerctl.timers[j] = timer;
    timerctl.next = timerctl.timers[0]->timeout;
    io_store_eflags(e);
}


void inthandler20(int esp) {
    int i,j;
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    if (timerctl.next > timerctl.count) {
        return;
    }
    
    for (i = 0; i < timerctl.using; i++) {
        if (timerctl.timers[i]->timeout > timerctl.count) {
            break;
        }
        timerctl.timers[i]->flags = 1;
        fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    
    timerctl.using -=i;
    
    for (j = 0; j < timerctl.using; j++) {
        timerctl.timers[j] = timerctl.timers[j+i];
    }
    
    if (timerctl.using > 0) {
        timerctl.next = timerctl.timers[0]->timeout;
    }else{
        timerctl.next = 0xffffffff;
    }
    
}

