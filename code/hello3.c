//
//  hello3.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/8.
//  Copyright © 2020 YI. All rights reserved.
//


void api_putchar(int c);
void api_putstr0(char *s);
void api_end(void);

int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int clo, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);

void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);

void api_end(void);

//char buf[150*50];

void HariMain(void){
//    api_putchar('Z');
//    api_putchar('h');
//    api_putchar('a');
//    api_putchar('I');
//    api_putchar('1');
    
//    for (; ; ) {
//        api_putchar('z');
//    }
    
    api_putstr0("hello, world\n");
    
    api_initmalloc();
    char *buf = api_malloc(150*100);
    int win = api_openwin(buf, 150, 100, -1, "zhello");
    api_boxfilwin(win, 6, 26, 141, 93 ,0);
//    api_putstrwin(win, 28, 28, 0,12, "hello,wolaile~~~");
    api_free(buf,150*100);
    
    api_point(win+1,75,59,3);
    api_refreshwin(win, 6, 26, 144, 94);
    
    api_end();
}
