//
//  hello3.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/8.
//  Copyright © 2020 YI. All rights reserved.
//

//color2.c
int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_initmalloc(void);
char *api_malloc(int size);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
int api_getkey(int mode);
void api_end(void);

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain(void)
{
    char *buf;
    int win, x, y;
    api_initmalloc();
    buf = api_malloc(144 * 164);
    win = api_openwin(buf, 144, 164, -1, "color2");
    for (y = 0; y < 128; y++) {
        for (x = 0; x < 128; x++) {
            buf[(x + 8) + (y + 28) * 144] = rgb2pal(x * 2, y * 2, 0, x, y);
        }
    }
    api_refreshwin(win, 8, 28, 136, 156);
    api_getkey(1); /* てきとうなキー入力を待つ */
    api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y)
{
    static int table[4] = { 3, 1, 0, 2 };
    int i;
    x &= 1; /* 偶数か奇数か */
    y &= 1;
    i = table[x + y * 2];    /* 中間色を作るための定数 */
    r = (r * 21) / 256;    /* これで 0〜20 になる */
    g = (g * 21) / 256;
    b = (b * 21) / 256;
    r = (r + i) / 4;    /* これで 0〜5 になる */
    g = (g + i) / 4;
    b = (b + i) / 4;
    return 16 + r + g * 6 + b * 36;
}










//#include <stdio.h>
//
//int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
//void api_putstrwin(int win, int x, int y, int col, int len, char *str);
//void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
//void api_initmalloc(void);
//char *api_malloc(int size);
//int api_getkey(int mode);
//int api_alloctimer(void);
//void api_inittimer(int timer, int data);
//void api_settimer(int timer, int time);
//void api_end(void);
//void api_beep(int tone);
//void api_refreshwin(int win, int x0, int y0, int x1, int y1);
//
//void HariMain(void)
//{
////    char *buf, s[12];
////    int win, timer, sec = 0, min = 0, hou = 0;
////    api_initmalloc();
////    buf = api_malloc(150 * 50);
////    win = api_openwin(buf, 150, 50, -1, "noodle");
////    timer = api_alloctimer();
////    api_inittimer(timer, 128);
////    for (;;) {
////        sprintf(s, "%5d:%02d:%02d", hou, min, sec);
////        api_boxfilwin(win, 28, 27, 115, 41, 7 /* 白 */);
////        api_putstrwin(win, 28, 27, 0 /* 黒 */, 11, s);
////        api_settimer(timer, 100);    /* 1秒間 */
////        if (api_getkey(1) != 128) {
////            break;
////        }
////        sec++;
////        if (sec == 60) {
////            sec = 0;
////            min++;
////            if (min == 60) {
////                min = 0;
////                hou++;
////            }
////        }
////    }
////    api_end();
//
//
////    beepup.c
//
////    int i, timer;
////    timer = api_alloctimer();
////    api_inittimer(timer, 128);
////    for (i = 20000; i <= 20000000; i += i / 100) {
////        /* 20KHz〜20Hz : 人間に聞こえる音の範囲 */
////        /* iは1%ずつ減らされていく */
////        api_beep(i);
////        api_settimer(timer, 1);        /* 0.01秒 */
////        if (api_getkey(1) != 128) {
////            break;
////        }
////    }
////    api_beep(0);
////    api_end();
//
//    //color.c
//    char *buf;
//    int win, x, y, r, g, b;
//    api_initmalloc();
//    buf = api_malloc(144 * 164);
//    win = api_openwin(buf, 144, 164, -1, "color");
//    for (y = 0; y < 128; y++) {
//        for (x = 0; x < 128; x++) {
//            r = x * 2;
//            g = y * 2;
//            b = 0;
//            buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
//        }
//    }
//    api_refreshwin(win, 8, 28, 136, 156);
//    api_getkey(1); /* てきとうなキー入力を待つ */
//    api_end();
//}
