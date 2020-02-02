#include <stdio.h>  //可以解决关于 sprintf 的警告

#define ADR_BOOTINFO 0xff0

// graphic.c 调色板和屏幕初始化相关
#define COL8_BLACK			0
#define COL8_RED			1
#define COL8_GREEN			2
#define COL8_YELLOW			3
#define COL8_BLUE			4
#define COL8_PURPLE			5
#define COL8_LIGHTBLUE		6
#define COL8_WHITE			7
#define COL8_GREY			8
#define COL8_DARKRED		9
#define COL8_DARKGREED		10
#define COL8_DARKYELLOW		11
#define COL8_DARKGRASS		12
#define COL8_DARKPURPLE		13
#define COL8_DARKBLUE		14
#define COL8_DARKGREY		15

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15



struct BOOTINFO {
	char cyls,leds;
	short vmode,scrnx,scrny;
	unsigned char * vram;
};

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen8(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram,int xsize,int c,int x0,int y0,int x1,int y1); //绘制矩形
void init_mouse_cursor8(char *mouse,int bc);
void putblock8_8(unsigned char *vram,int vxsize,int pxsize,int pysize,
    int px0,int py0,char *buf,int bxsize);  //把鼠标的buff写到显存中
void putfonts8_asc(unsigned char *vram, int xsize, int x, int  y, int c, char *s); //显示字符串


// naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
char io_in8(int port);
void io_out8(int addr, int data);
int io_load_eflags(void);
void io_store_eflags(int data);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_inthandler27(void);

int load_cr0(void);
void store_cr0(int data);



// dsctbl.c gdt idt 相关
struct SEGMENT_DESCRITOR {
    short limit_low,base_low;
    char base_mid,access_right,limit_high,base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};


void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRITOR *sd,int limit,int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


// int.c
void init_pic(void);
#define PIC0_ICW1        0x0020
#define PIC0_OCW2        0x0020
#define PIC0_IMR        0x0021
#define PIC0_ICW2        0x0021
#define PIC0_ICW3        0x0021
#define PIC0_ICW4        0x0021
#define PIC1_ICW1        0x00a0
#define PIC1_OCW2        0x00a0
#define PIC1_IMR        0x00a1
#define PIC1_ICW2        0x00a1
#define PIC1_ICW3        0x00a1
#define PIC1_ICW4        0x00a1

//#define KEYBUFLEN 32
//struct KEYBUF {
//    char data[KEYBUFLEN];
//    int right,left,len;
//};


// fifo8.c
struct FIFO8 {
    unsigned char *buf;
    int left,right,size,free,flags;
};

void fifo8_init(struct FIFO8 *fifo,int size,unsigned char *buf);
void fifo8_put(struct FIFO8 *fifo,char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);


// mouse & keyboard

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void wait_KBC_sendready(void);


// memory.c

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
unsigned int memman_alloc_4k(struct MEMMAN *man,unsigned int size);
int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size);
int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size);


// sheet.c


struct SHEET {
    unsigned char *buf;
    int bxsize,bysize,vx0,vy0,col_inv,height,flags;
};

#define MAX_SHEETS 256

struct SHTCTL {
    unsigned char *vram;
    int xsize,ysize,top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};


struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize,int col_inv);
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height) ;
void sheet_refresh(struct SHTCTL *ctl);
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0,int vy0);
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);


