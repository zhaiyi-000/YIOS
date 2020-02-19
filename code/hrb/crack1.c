//
//  crack1.c
//  YIOS_xcode
//
//  Created by 上工 on 2020/2/8.
//  Copyright © 2020 YI. All rights reserved.
//

void api_end(void);

void HariMain(void)
{
    *((char *) 0x00102600) = 0;
    api_end();
}

