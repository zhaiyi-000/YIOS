TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -I../ -Os -Wall -quiet
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
	$(MAKE) type.hrb
	$(MAKE) iroha.hrb
	$(MAKE) notrec.hrb
	$(MAKE) bball.hrb
	$(MAKE) invader.hrb
	


%.gas : %.c ../apilib.h Makefile
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
	
type.bim : type.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:type.bim map:type.map type.obj a_nask.obj
type.hrb : type.bim Makefile
	$(BIM2HRB) type.bim type.hrb 0
	
	
iroha.bim : iroha.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:iroha.bim map:iroha.map iroha.obj a_nask.obj
iroha.hrb : iroha.bim Makefile
	$(BIM2HRB) iroha.bim iroha.hrb 0
	
notrec.bim : notrec.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:notrec.bim map:notrec.map notrec.obj a_nask.obj
notrec.hrb : notrec.bim Makefile
	$(BIM2HRB) notrec.bim notrec.hrb 0
	
bball.bim : bball.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bball.bim map:bball.map bball.obj a_nask.obj
bball.hrb : bball.bim Makefile
	$(BIM2HRB) bball.bim bball.hrb 0
	
invader.bim : invader.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:invader.bim stack:90k map:invader.map invader.obj a_nask.obj
invader.hrb : invader.bim Makefile
	$(BIM2HRB) invader.bim invader.hrb 0
	
	


clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.img
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) haribote.sys
