TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = ./haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


default :
	$(MAKE) hello.hrb
	$(MAKE) crack2.hrb
	$(MAKE) hello2.hrb
	$(MAKE) a.hrb
	$(MAKE) color2.hrb
	$(MAKE) crack1.hrb


%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

	
hello.hrb : hello.nas Makefile
	$(NASK) hello.nas hello.hrb hello.lst

crack2.hrb : crack2.nas Makefile
	$(NASK) crack2.nas crack2.hrb crack2.lst
	
hello2.hrb : hello2.nas Makefile
	$(NASK) hello2.nas hello2.hrb hello2.lst
	
a.bim : a.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:a.bim map:a.map a.obj a_nask.obj
a.hrb : a.bim Makefile
	$(BIM2HRB) a.bim a.hrb 0
	
	
color2.bim : color2.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color2.bim stack:2k map:color2.map color2.obj a_nask.obj
color2.hrb : color2.bim Makefile
	$(BIM2HRB) color2.bim color2.hrb 100k
	
	
crack1.bim : crack1.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack1.bim map:crack1.map crack1.obj a_nask.obj
crack1.hrb : crack1.bim Makefile
	$(BIM2HRB) crack1.bim crack1.hrb 0
	
	


clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.img
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) haribote.sys
