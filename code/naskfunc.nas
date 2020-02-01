[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]
	GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL _load_gdtr,_load_idtr
    GLOBAL _asm_inthandler21,_asm_inthandler2c,_asm_inthandler27
    GLOBAL _load_cr0,_store_cr0
    GLOBAL _memtest_sub

	
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

_load_gdtr:
    mov ax,[esp+4]
    mov [esp+6],ax
    lgdt [esp+6]
    ret


_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET



    extern _inthandler21,_inthandler2c,_inthandler27
_asm_inthandler21:
    push ds
    push es
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler21
    pop eax
    popad
    pop es
    pop ds
    iret
    
_asm_inthandler2c:
    push ds
    push es
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler2c
    pop eax
    popad
    pop es
    pop ds
    iret


_asm_inthandler27:
    push ds
    push es
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler27
    pop eax
    popad
    pop es
    pop ds
    iret


_load_cr0:
    mov eax,cr0
    ret
    
    
_store_cr0:
    mov eax,[esp+4]
    mov cr0,eax
    ret


_memtest_sub:
    push ebx
    push esi
    push edi
    
    mov esi,0xaa55aa55
    mov edi,0x55aa55aa
    
    mov eax,[esp+12+4]
    
mts_loop:
    mov edx,eax
    add edx,0xffc

    mov ebx,[edx] ;old
    mov [edx],esi
    xor dword [edx],0xffffffff
    cmp [edx],edi
    jne mts_fin
    xor dword [edx],0xffffffff
    cmp [edx],esi
    jne mts_fin
    mov [edx],ebx
    
    add eax,0x1000
    cmp eax,[esp+12+8]
    jbe mts_loop
    
    pop edi
    pop esi
    pop ebx
    ret
    
    
    
    
mts_fin:
    mov [edx],ebx
    pop edi
    pop esi
    pop ebx
    ret

