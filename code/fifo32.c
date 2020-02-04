//
//  fifo32.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/30.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

void fifo32_init(struct FIFO32 *fifo,int size,int *buf){
    fifo->buf = buf;
    fifo->left = 0;
    fifo->right = 0;
    fifo->size = size;
    fifo->free = size;
    fifo->flags = 0;
}

void fifo32_put(struct FIFO32 *fifo,int data){
    fifo->buf[fifo->right] = data;
    fifo->right++;
    if (fifo->right==fifo->size) {
        fifo->right = 0;
    }
    fifo->free--;
}
int fifo32_get(struct FIFO32 *fifo){
    int data =fifo->buf[fifo->left];
    fifo->left++;
    if (fifo->left==fifo->size) {
        fifo->left = 0;
    }
    fifo->free++;
    return data;
}
int fifo32_status(struct FIFO32 *fifo){
    return fifo->size-fifo->free;
}

