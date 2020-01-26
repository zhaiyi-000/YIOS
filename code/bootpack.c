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

void io_hlt();
void io_cli();
void io_out8(int addr, int data);
int io_load_eflags();
void io_store_eflags(int data);


void init_palette();
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram,int xsize,int c,int x0,int y0,int x1,int y1);

void HariMain(){

	init_palette();

	unsigned char *vram = (unsigned char *)0xa0000;

	boxfill8(vram,320,COL8_RED,20,20,120,120);
	boxfill8(vram,320,COL8_YELLOW,70,50,170,150);
	boxfill8(vram,320,COL8_GREEN,120,80,220,180);

	for(;;){
		io_hlt();
	}
}

void init_palette(){
	static unsigned char rgb[16*3] = {
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};

	set_palette(0,15,rgb);
}

void set_palette(int start, int end, unsigned char *rgb) {
	int i;
	int flag = io_load_eflags();
	io_cli();
	io_out8(0x3c8,start);
	for(i = start;i<=end;i++){
		io_out8(0x3c9,rgb[0]/4);
		io_out8(0x3c9,rgb[1]/4);
		io_out8(0x3c9,rgb[2]/4);
		rgb+=3;
	}
	io_store_eflags(flag);
}

void boxfill8(unsigned char *vram,int xsize,int c,int x0,int y0,int x1,int y1){
	int x,y;
	for (y=y0;y<=y1;y++) {
		for (x=x0;x<=x1;x++) {
			vram[y*xsize+x] = c;
		}
	}
}




