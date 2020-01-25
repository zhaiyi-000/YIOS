	org 0xc200
	
	mov ah,0
	mov al,0x13
	int 0x10

fin:
	hlt
	jmp fin




	