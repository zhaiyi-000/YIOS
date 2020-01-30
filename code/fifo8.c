//
//  fifo8.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/1/30.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo,int size,unsigned char *buf){
    fifo->buf = buf;
    fifo->left = 0;
    fifo->right = 0;
    fifo->size = size;
    fifo->free = size;
    fifo->flags = 0;
}

void fifo8_put(struct FIFO8 *fifo,char data){
    fifo->buf[fifo->right] = data;
    fifo->right++;
    if (fifo->right==fifo->size) {
        fifo->right = 0;
    }
    fifo->free--;
}
int fifo8_get(struct FIFO8 *fifo){
    int data =fifo->buf[fifo->left];
    fifo->left++;
    if (fifo->left==fifo->size) {
        fifo->left = 0;
    }
    fifo->free++;
    return data;
}
int fifo8_status(struct FIFO8 *fifo){
    return fifo->size-fifo->free;
}

