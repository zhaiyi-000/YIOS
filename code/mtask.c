//
//  mtask.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/5.
//  Copyright © 2020 YI. All rights reserved.
//

#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

void task_idle(void);

struct TASK *task_now(void){
    struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
    return tl->tasks[tl->now];
}

void task_add(struct TASK *task){
    struct TASKLEVEL *tl = &taskctl->level[task->level];
    tl->tasks[tl->running] = task;
    tl->running++;
    task->flags = 2;
}

void task_remove(struct TASK *task){
    int i;
    struct TASKLEVEL *tl = &taskctl->level[task->level];
    for (i = 0; i < tl->running; i++) {
        if (tl->tasks[i]==task) {
            break;
        }
    }
    tl->running--;
    if (i < tl->now) {
        tl->now--;
    }
    if (tl->now >= tl->running) {
        tl->now = 0;
    } // 异常,我也不知道什么时候会出现
    task->flags = 1;  //休眠中
    
    for (; i < tl->running; i++) {
        tl->tasks[i] = tl->tasks[i+1];
    }
}

// 用来在任务切换时决定接下来切换到哪个level
void task_switchsub(void){
    int i;
    for (i =0; i < MAX_TASKLEVELS; i++) {
        if (taskctl->level[i].running > 0) {
            break;
        }
    }
    taskctl->now_lv = i;
    taskctl->lv_change = 0;  //不知道这个属性用来干啥
}

struct TASK *task_init(struct MEMMAN *memman){
    int i;
    struct TASK *task, *idle;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
    
    for (i = 0; i < MAX_TASKS; i++) {
        taskctl->task0[i].flags = 0;
        taskctl->task0[i].sel = (TASK_GDT0+i)*8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->task0[i].tss, AR_TSS32);
    }
        
    for (i = 0; i < MAX_TASKLEVELS; i++) {
        taskctl->level[i].running = 0;
        taskctl->level[i].now = 0;
    }
    task = task_alloc();
    
    task->flags = 2;   //2表示正在运行  //0表示未分配  //1表示睡眠
    task->priority = 2;
    task->level = 0;
    task_add(task);
    task_switchsub();
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, task->priority);
    
    
    
    idle = task_alloc();
    idle->tss.esp = memman_alloc_4k(memman, 64*1024)+64*1024-8;
    idle->tss.eip = (int)task_idle;
    idle->tss.es = 1*8;
    idle->tss.cs = 2*8;
    idle->tss.ss = 1*8;
    idle->tss.ds = 1*8;
    idle->tss.fs = 1*8;
    idle->tss.gs = 1*8;
    task_run(idle,MAX_TASKLEVELS-1,1);  //并不会立即发生切换
    
    return task;
}

struct TASK *task_alloc(void){
    int i;
    struct TASK *task;
    for (i = 0; i < MAX_TASKS; i++) {
        if (taskctl->task0[i].flags==0) {
            task = &taskctl->task0[i];
            task->flags = 1;
            task->tss.eflags = 0x202; //if = 1;
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.edx = 0;
            task->tss.ebx = 0;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.es = 0;
            task->tss.ds = 0;
            task->tss.fs = 0;
            task->tss.gs = 0;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            return task;
        }
    }
    return 0;
}

void task_run(struct TASK *task, int level, int priority){ //可以修改level,可以修改priority
    if (level < 0) {
        level = task->level; //和下面这个用法一样
    }
    
    if (priority > 0) {//如果priority==0,只用来唤醒,不改变优先级
        task->priority = priority;
    }
    
    if (task->flags==2 && task->level != level) {  //
        task_remove(task);
    }
    
    if (task->flags != 2) {
        task->level = level;
        task_add(task);
    }
    
    taskctl->lv_change = 1;
}
void task_switch(void){
    struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
    struct TASK *new_task, *now_task = tl->tasks[tl->now];
    tl->now++;
    if (tl->now==tl->running) {
        tl->now = 0;
    }
    if (taskctl->lv_change != 0) {
        task_switchsub();
        tl = &taskctl->level[taskctl->now_lv];
    }
    new_task = tl->tasks[tl->now];
    timer_settime(task_timer, new_task->priority);
    if (new_task != now_task) {
        farjmp(0, new_task->sel);
    }
}

void task_sleep(struct TASK *task){
    
    struct TASK *now_task;
    if (task->flags==2) {
        now_task = task_now();
        task_remove(task);
        if (task==now_task) {
            task_switchsub();
            now_task = task_now();
            farjmp(0, now_task->sel);
        }
    }
}

void task_idle(void) {
    for (; ; ) {
        io_hlt();
    }
}
