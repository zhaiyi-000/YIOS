//
//  fifo32.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/30.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

void fifo32_init(struct FIFO32 *fifo,int size,int *buf, struct TASK *task){
    fifo->buf = buf;
    fifo->p = 0;
    fifo->q = 0;
    fifo->size = size;
    fifo->free = size;
    fifo->flags = 0;
    fifo->task = task;
}

int fifo32_put(struct FIFO32 *fifo,int data){
    fifo->buf[fifo->q] = data;
    fifo->q++;
    if (fifo->q==fifo->size) {
        fifo->q = 0;
    }
    fifo->free--;
    
    if (fifo->task != 0) {
        if (fifo->task->flags !=2) {
            task_run(fifo->task,0);
        }
    }
    
    return 0;
}
int fifo32_get(struct FIFO32 *fifo){
    int data =fifo->buf[fifo->p];
    fifo->p++;
    if (fifo->p==fifo->size) {
        fifo->p = 0;
    }
    fifo->free++;
    return data;
}
int fifo32_status(struct FIFO32 *fifo){
    return fifo->size-fifo->free;
}

