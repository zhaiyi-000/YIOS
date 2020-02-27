//
//  notrec.c
//  YIOS_xcode
//
//  Created by zhaiyi on 2020/2/27.
//  Copyright © 2020 YI. All rights reserved.
//

#include "apilib.h"

void HariMain(void){
    int win;
    char buf[150 * 70];
    win = api_openwin(buf, 150, 70, 255, "notrec");
    api_boxfilwin(win,   0, 50,  34, 69, 255);
    api_boxfilwin(win, 115, 50, 149, 69, 255);
    api_boxfilwin(win,  50, 30,  99, 49, 255);
    for (;;) {
        if (api_getkey(1) == 0x0a) {
            break;
        }
    }
    api_end();
}
