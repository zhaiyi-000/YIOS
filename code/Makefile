OBJS_BOOTPACK = naskfunc.obj hankaku.obj bootpack.obj dsctbl.obj graphic.obj int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj timer.obj mtask.obj window.obj console.obj file.obj

TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

default :
	$(MAKE) img

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst
asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin
hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile \
		hrb/hello.hrb hrb/hello2.hrb hrb/a.hrb hrb/color2.hrb hrb/crack1.hrb hrb/crack2.hrb \
		hrb/type.hrb hrb/iroha.hrb hrb/notrec.hrb hrb/bball.hrb hrb/invader.hrb
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:make.bat to:@: \
		copy from:hrb/hello.hrb to:@: \
		copy from:hrb/hello2.hrb to:@: \
		copy from:hrb/a.hrb to:@: \
		copy from:hrb/color2.hrb to:@: \
		copy from:hrb/crack1.hrb to:@: \
		copy from:hrb/crack2.hrb to:@: \
		copy from:hrb/type.hrb to:@: \
		copy from:hrb/iroha.hrb to:@: \
		copy from:hrb/notrec.hrb to:@: \
		copy from:hrb/bball.hrb to:@: \
		copy from:hrb/invader.hrb to:@: \
		copy from:HZK16.fnt to:@: \
		imgout:haribote.img

cp :
	xcopy "Z:\desktop\YIOS_xcode\YIOS\code" "C:\Users\zhaiyi\Desktop\YIOS\code" /e/i/h/y
	xcopy "C:\Users\zhaiyi\Desktop\YIOS\code\haribote.img" "Z:\desktop" /e/i/h/y

img :
	xcopy "Z:\desktop\YIOS_xcode\YIOS\code" "C:\Users\zhaiyi\Desktop\YIOS\code" /e/i/h/y
	$(MAKE) -C hrb
	$(MAKE) haribote.img
	xcopy "C:\Users\zhaiyi\Desktop\YIOS\code\haribote.img" "Z:\desktop" /e/i/h/y

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.img
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) haribote.sys
	$(MAKE) -C hrb clean

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
