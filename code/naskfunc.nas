[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]
	GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL _load_gdtr,_load_idtr

	
[SECTION .text]

_io_hlt:
	hlt
	ret

_io_cli:
	cli
	ret

_io_sti:
	sti
	ret

_io_stihlt:
	sti
	hlt
	ret

_io_in8:
	mov edx,[esp+4]
	mov eax,0
	in al,dx
	ret

_io_in16:
	mov edx,[esp+4]
	mov eax,0
	in ax,dx
	ret

_io_in32:
	mov edx,[esp+4]
	in eax,dx
	ret

_io_out8:
	mov edx,[esp+4]
	mov al,[esp+8]
	out dx,al
	ret

_io_out16:
	mov edx,[esp+4]
	mov ax,[esp+8]
	out dx,ax
	ret

_io_out32:
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,eax
	ret

_io_load_eflags:
	pushfd
	pop eax
	ret

_io_store_eflags:
	mov eax,[esp+4]
	push eax
	popfd
	ret




_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET
