	
CYLS equ 0xff0
LEDS equ 0xff1
VMODE equ 0xff2
SCRNX equ 0xff4
SCRNY equ 0xff6
VRAM equ 0xff8

	org 0xc200
	
	mov ah,0
	mov al,0x13
	int 0x10

	mov byte [VMODE],8
	mov word [SCRNX],320
	mov word [SCRNY],200
	mov dword [VRAM],0xa0000

	mov ah,2
	int 0x16
	mov [LEDS],al

fin:
	hlt
	jmp fin




	