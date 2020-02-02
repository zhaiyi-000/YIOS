//
//  memory.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/2.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"




#define EFLAGS_AC_BIT 0x40000
#define CR0_CACHE_DISABLE 0x60000000




unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg,cr0,i;
    
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    
    if ((eflg & EFLAGS_AC_BIT)!=0){
        flg486 = 1;
    }
    
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    
    if (flg486==1) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    i = memtest_sub(start,end);
    
    if (flg486 == 1) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    return i;
}



void memman_init(struct MEMMAN *man){
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
}
unsigned int memman_total(struct MEMMAN *man){
    int i,total = 0;
    for (i = 0; i < man->frees; i++) {
        total += man->free[i].size;
    }
    return total;
}
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size) {
    int i;
    unsigned int a;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size>= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            
            if (man->free[i].size==0) {
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return -1;
}
int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size){
    int i,j;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    
//    i-1 < addr < i
    
    if (i >0 && man->free[i-1].addr + man->free[i-1].size == addr){
        //前面有且可合并
        man->free[i-1].size += size;
        
        if (i < man->frees && addr + size == man->free[i].addr) {
            man->frees--;
            man->free[i-1].size += man->free[i].size;
            for (j = i; j < man->frees; j++) {
                man->free[j] = man->free[j+1];
            }
        }
        
        return 0;
    }
    
    if (i < man->frees && addr + size == man->free[i].addr) {
        //后面有且可合并
        man->free[i].addr -= size;
        man->free[i].size += size;
        
        return 0;
    }
    
    if (man->frees < MEMMAN_FREES) {
        for (j = man->frees; j > i; j--) {  //这个地方要倒着来,不然有bug
            man->free[j] = man->free[j-1];
        }
        man->frees++;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        
        
        man->free[i].addr = addr;
        man->free[i].size = size;
        
        return 0;
    }
    
    man->losts++;
    man->lostsize+= size;
    
    return -1;
}



unsigned int memman_alloc_4k(struct MEMMAN *man,unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;
    return memman_alloc(man, size);
}
int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size){
    size = (size + 0xfff) & 0xfffff000;
    return memman_free(man, addr,size);
}
