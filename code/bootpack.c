#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

#define MEMMAN_FREES 4090

struct FREEINFO {
    unsigned int addr, size;
};
struct MEMMAN {
    int frees, maxfrees,lostsize,losts;
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size);
int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size);


void yiPrintf(){
    struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 20, 310, 36);
    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 20, COL8_YELLOW, "wo lai~~~~~");
}

void HariMain(){

    char s[100];
	struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
	unsigned char *vram = bInfo->VRAM;
	int xsize = bInfo->SCRNX;
	int ysize = bInfo->SCRNY;
    char keyBuf__[32];
    char mouseBuf__[128];
    fifo8_init(&keyfifo, 32, keyBuf__);
    fifo8_init(&mousefifo, 128, mouseBuf__);

    init_gdtidt();
    init_pic();
    io_sti();   //这个地方产生了一个bug,调试了好久.....
	init_palette();
	init_screen(vram, xsize, ysize);

    
    struct MOUSE_DEC mdec;
    init_keyboard();
    enable_mouse(&mdec);
    
	//logo
    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 0, 310, 18);
	putfont8_asc(vram, xsize,1,1,COL8_YELLOW,"HELLO YIOS");
	putfont8_asc(vram, xsize,0,0,COL8_YELLOW,"HELLO YIOS");


	//鼠标
	char mouse[256];
    int mx = 160,my = 100;
	init_mouse_cursor8(mouse,COL8_008484);
	putblock8_8(vram,xsize,16,16,mx,my,mouse,16);
    
    // 检查内存
    struct MEMMAN *memman = (struct MEMMAN *)0x3c0000;  //#define MEMMAN_ADDR 0x3c0000
    unsigned int memtotal = memtest(0x400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x1000, 0x9e000);
    memman_free(memman, 0x400000, memtotal-0x400000);
    
    
    // free 29304=632k(1m-4k(0开头的BIOS)-4k(后面的BIOS)-384k(0xa0000-0xaffff  显存用的地方 64k  后面的320我就不知道是干啥的了))+28m
    sprintf(s, "[total %dM, free %dK]",memtotal/1024/1024,memman_total(memman)/1024);
    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 100, 310, 115);
    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 100, COL8_YELLOW, s);
    

    unsigned char data;
	for(;;){
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo)==0) {
            io_stihlt();
        }else{
            if (fifo8_status(&keyfifo)!=0) {
                data = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "jianpan %02X",data);
                
                struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
                boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 40, 310, 56);
                putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 40, COL8_YELLOW, s);
            }else if(fifo8_status(&mousefifo)!=0){
                data = fifo8_get(&mousefifo);
                io_sti();
                
                if (mouse_decode(&mdec, data) != 0) {
                    
                    sprintf(s, "lcr %8X %8X",mdec.x,mdec.y);
                    
                    if ((mdec.btn & 0x1)!=0) {
                        s[0] = 'L';
                    }
                    if ((mdec.btn & 0x2)!=0) {
                        s[2] = 'R';
                    }
                    if ((mdec.btn & 0x4)!=0) {
                        s[1] = 'C';
                    }
                    
                    struct BootInfo *bInfo = (struct BootInfo *)ADR_BOOTINFO;
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 60, 310, 76);
                    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 60, COL8_YELLOW, s);
                    
                    //消除鼠标
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_008484, mx, my, mx+15, my+15);
                    
                    mx+=mdec.x;
                    my+=mdec.y;
                    
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > bInfo->SCRNX - 16) {
                        mx = bInfo->SCRNX - 16;
                    }
                    if (my > bInfo->SCRNY - 16) {
                        my = bInfo->SCRNY - 16;
                    }
                    
                    sprintf(s, "[zuobiao %3d %3d]",mx,my);
                    boxfill8(bInfo->VRAM, bInfo->SCRNX, COL8_RED, 0, 80, 310, 96);
                    putfont8_asc(bInfo->VRAM, bInfo->SCRNX, 0, 80, COL8_YELLOW, s);
                    putblock8_8(bInfo->VRAM, bInfo->SCRNX, 16, 16, mx, my, mouse, 16);
                }
            }
        }
	}
    
    
}

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
