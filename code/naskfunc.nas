[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]
	GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL _load_gdtr,_load_idtr
    GLOBAL _asm_inthandler21,_asm_inthandler2c,_asm_inthandler27,_asm_inthandler20
    GLOBAL _load_cr0,_store_cr0
    GLOBAL _memtest_sub
    GLOBAL _load_tr,_farjmp,_farcall
    GLOBAL _asm_hrb_api,_start_app
    
    EXTERN _inthandler21,_inthandler2c,_inthandler27,_inthandler20,_hrb_api

	
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



    
_asm_inthandler21:
    push ds
    push es
    pushad
    
    mov ax,ss
    cmp ax,1*8
    jne .from_app
    
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
    
.from_app:
    mov eax,1*8
    mov es,ax
    mov ecx,[0xfe4]
    add ecx,-8
    mov [ecx+4],ss
    mov [ecx],esp
    mov ss,ax
    mov es,ax
    mov esp,ecx
    call _inthandler20
    pop ecx
    pop eax
    mov ss,ax
    mov esp,ecx
    popad
    pop ds
    pop es
    iretd
    
_asm_inthandler2c:
    push ds
    push es
    pushad
    
    mov ax,ss
    cmp ax,1*8
    jne .from_app
    
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
    
.from_app:
    mov eax,1*8
    mov es,ax
    mov ecx,[0xfe4]
    add ecx,-8
    mov [ecx+4],ss
    mov [ecx],esp
    mov ss,ax
    mov es,ax
    mov esp,ecx
    call _inthandler2c
    pop ecx
    pop eax
    mov ss,ax
    mov esp,ecx
    popad
    pop ds
    pop es
    iretd


_asm_inthandler27:
    push ds
    push es
    pushad
    
    mov ax,ss
    cmp ax,1*8
    jne .from_app
    
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
    
.from_app:
    mov eax,1*8
    mov es,ax
    mov ecx,[0xfe4]
    add ecx,-8
    mov [ecx+4],ss
    mov [ecx],esp
    mov ss,ax
    mov es,ax
    mov esp,ecx
    call _inthandler27
    pop ecx
    pop eax
    mov ss,ax
    mov esp,ecx
    popad
    pop ds
    pop es
    iretd
    
_asm_inthandler20:
    push ds
    push es
    pushad
    
    mov ax,ss
    cmp ax,1*8
    jne .from_app
    
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler20
    pop eax
    popad
    pop es
    pop ds
    iret
    
.from_app:
    mov eax,1*8
    mov es,ax
    mov ecx,[0xfe4]
    add ecx,-8
    mov [ecx+4],ss
    mov [ecx],esp
    mov ss,ax
    mov es,ax
    mov esp,ecx
    call _inthandler20
    pop ecx
    pop eax
    mov ss,ax
    mov esp,ecx
    popad
    pop ds
    pop es
    iretd


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


_load_tr:
    ltr [esp+4]
    ret


_farjmp:
    jmp far [esp+4]
    ret
    
_farcall:
    call far [esp+4]
    ret


_asm_hrb_api:    ;0x40会调用到这里
    ;sti
    push ds
    push es
    pushad
    
    mov eax,1*8
    mov ds,ax
    mov ecx,[0xfe4]
    add ecx,-40
    mov [ecx+32],esp
    mov [ecx+36],ss
    
    mov edx,[esp]
    mov ebx,[esp+4]
    mov [ecx],edx
    mov [ecx+4],ebx
    
    mov edx,[esp+8]
    mov ebx,[esp+12]
    mov [ecx+8],edx
    mov [ecx+12],ebx
    
    mov edx,[esp+16]
    mov ebx,[esp+20]
    mov [ecx+16],edx
    mov [ecx+20],ebx
    
    mov edx,[esp+24]
    mov ebx,[esp+28]
    mov [ecx+24],edx
    mov [ecx+28],ebx
    
    mov es,ax
    mov ss,ax
    mov esp,ecx
    sti
    
    call _hrb_api
    mov ecx,[esp+32]
    mov eax,[esp+36]
    cli
    mov ss,ax
    mov esp,ecx
    popad
    pop es
    pop ds
    iretd
    
    
    pushad
    call _hrb_api
    add esp,32
    popad
    iretd  ;这个命令自动执行sti


; 从操作系统调用到用户程序
_start_app:        ; void start_app(int eip, int cs, int esp, int ds);
    pushad
    mov eax,[esp+36]
    mov ecx,[esp+40]
    mov edx,[esp+44]
    mov ebx,[esp+48]
    mov [0xfe4],esp
    ;为什么不把系统栈保存在用户栈?
    ;因为所有的app都要使用这个系统栈,只有一个系统栈,所有的app都去[0xfe4]这个地方取
    
    cli
    mov es,bx
    mov ss,bx
    mov ds,bx
    mov fs,bx
    mov gs,bx
    mov esp,edx
    sti
    push ecx
    push eax
    call far [esp]

;应用程序返回
    mov eax,1*8
    cli
    mov es,ax
    mov ss,ax
    mov ds,ax
    mov fs,ax
    mov gs,ax
    mov esp,[0xfe4]
    sti
    popad
    ret
