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



struct BootInfo {
	char CYLS,LEDS;
	short VMODE,SCRNX,SCRNY;
	unsigned char * VRAM;
};

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram,int xsize,int c,int x0,int y0,int x1,int y1); //绘制矩形
void init_mouse_cursor8(char *mouse,int bc);
void putblock8_8(unsigned char *vram,int vxsize,int pxsize,int pysize,
    int px0,int py0,char *buf,int bxsize);  //把鼠标的buff写到显存中
void putfont8_asc(unsigned char *vram, int xsize, int x, int  y, int c, char *s); //显示字符串


// naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_sti(void);
char io_in8(int port);
void io_out8(int addr, int data);
int io_load_eflags(void);
void io_store_eflags(int data);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_inthandler27(void);



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
