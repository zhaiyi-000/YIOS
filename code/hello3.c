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
    
    api_end();
}
