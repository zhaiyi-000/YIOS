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
char buf[150*50];

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
    
    int win = api_openwin(buf, 150, 50, -1, "zhello");
    api_end();
}
