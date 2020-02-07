#include "bootpack.h"

void init_gdtidt(void)
{
	
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x270000;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) 0x0026f800;
    int i;

	/* GDTの初期化 */
    for (i = 0; i < 8192; i++) {
        set_segmdesc(gdt+i, 0, 0, 0);
    }
    set_segmdesc(gdt+1, 0xffffffff, 0, 0x4092);
    set_segmdesc(gdt+2, 0x7ffff, 0x280000, 0x409a);
    load_gdtr(0xffff, 0x270000); //gdt表
	

	/* IDTの初期化 */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);
    
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 16, 0x8e);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 16, 0x8e);
    set_gatedesc(idt + 0x27, (int)asm_inthandler27, 16, 0x8e);
    set_gatedesc(idt + 0x20, (int)asm_inthandler20, 16, 0x8e);
    set_gatedesc(idt + 0x40, (int)asm_hrb_api, 16, 0x8e);
    
	return;
}


// md 这个limit如果申请成int,在VMware中启动不了....    ccccccc   调了3个钟头.......................
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
	if (limit > 0xfffff) {
        limit/=0x1000;
        ar |= 0x8000;
	}
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16)&0xf)|((ar >> 8)&0xf0);
    sd->base_high = (base >> 24) & 0xff;
}


void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
