//
//  timer.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/3.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"


struct TIMERCTL timerctl;

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
            timerctl.timers0[i].flags2 = 0;
            return timerctl.timers0+i;
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer){
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data) {
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout){
    int e;
    struct TIMER *t,*s;
    timer->timeout = timeout + timerctl.count;
    timer->flags = 2;
    
    e = io_load_eflags();
    io_cli();
    
    timerctl.using++;
    if (timerctl.using==1) {
        timerctl.t0 = timer;
        timer->next = 0;
        timerctl.next = timer->timeout;
        io_store_eflags(e);
        return;
    }
    
    t = timerctl.t0;
    if (timer->timeout <= t->timeout) {
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next = timer->timeout;
        io_store_eflags(e);
        return;
    }
    
    for (; ; ) {
        s = t;
        t = t->next;
        if (t==0) {
            s->next = timer;
            timer->next = 0;
            io_store_eflags(e);
            return;
        }
        
        if (timer->timeout<= t->timeout) {
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
}
extern struct TIMER *task_timer;

void inthandler20(int* esp) {
    int i,ts = 0;
    struct TIMER *timer;
    
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    if (timerctl.next > timerctl.count) {
        return;
    }
    timer = timerctl.t0;
    
    for (i = 0; i < timerctl.using; i++) {
        if (timer->timeout > timerctl.count) {
            break;
        }
        timer->flags = 1;
        
        if (timer != task_timer) {
            fifo32_put(timer->fifo, timer->data);
        }else{
            ts = 1;
        }
        
        timer = timer->next;
    }
    
    timerctl.using -=i;
    timerctl.t0 = timer;
    
    if (timerctl.using > 0) {
        timerctl.next = timerctl.t0->timeout;
    }else{
        timerctl.next = 0xffffffff;
    }
    
    if (ts!=0) {
        task_switch();
    }
    
}


int timer_cancel(struct TIMER *timer){
    int e;
    struct TIMER *t;
    e = io_load_eflags();
    io_cli();
    if (timer->flags == 2) {
        if (timer==timerctl.t0) {
            t = timer->next;
            timerctl.t0 = t;
            timerctl.next = t->timeout;
        }else{
            t = timerctl.t0;
            for (; ; ) {
                if (t->next == timer) {
                    break;
                }
                t = t->next;
            }
            t->next = timer->next;
        }
        timer->flags = 1;
        io_store_eflags(e);
        return 1;
    }
    
    io_store_eflags(e);
    return 0;
}

void timer_cancelall(struct FIFO32 *fifo)
{
    int e, i;
    struct TIMER *t;
    e = io_load_eflags();
    io_cli();    /* 設定中にタイマの状態が変化しないようにするため */
    for (i = 0; i < MAX_TIMER; i++) {
        t = &timerctl.timers0[i];
        if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {//根据这个来取消的
            timer_cancel(t);
            timer_free(t);
        }
    }
    io_store_eflags(e);
    return;
}
