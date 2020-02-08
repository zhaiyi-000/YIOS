[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]
	GLOBAL _io_hlt,_io_cli,_io_sti,_io_stihlt
	GLOBAL _io_in8,_io_in16,_io_in32
	GLOBAL _io_out8,_io_out16,_io_out32
	GLOBAL _io_load_eflags,_io_store_eflags
	GLOBAL _load_gdtr,_load_idtr
    GLOBAL _asm_inthandler21,_asm_inthandler2c,_asm_inthandler27,_asm_inthandler20,_asm_inthandler0d,_asm_inthandler0c
    GLOBAL _load_cr0,_store_cr0
    GLOBAL _memtest_sub
    GLOBAL _load_tr,_farjmp,_farcall
    GLOBAL _asm_hrb_api,_start_app
    
    EXTERN _hrb_api
    EXTERN _inthandler21,_inthandler2c,_inthandler27,_inthandler20,_inthandler0d,_inthandler0c

	
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
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler21
    pop eax
    popad
    pop ds
    pop es
    iret
    

    
_asm_inthandler2c:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler2c
    pop eax
    popad
    pop ds
    pop es
    iret
    


_asm_inthandler27:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler27
    pop eax
    popad
    pop ds
    pop es
    iret
    

    
_asm_inthandler20:
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler20
    pop eax
    popad
    pop ds
    pop es
    iret
    
    
_asm_inthandler0d:
    sti ;打开中断

    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler0d
    cmp eax,0
    jne end_app
    pop eax
    popad
    pop ds
    pop es
    
    add esp,4
    
    iret
    
    
_asm_inthandler0c:
    sti ;打开中断
    
    push es
    push ds
    pushad
    mov eax,esp
    push eax
    mov ax,ss
    mov ds,ax
    mov es,ax
    call _inthandler0c
    cmp eax,0
    jne end_app
    pop eax
    popad
    pop ds
    pop es

    add esp,4
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


_load_tr:
    ltr [esp+4]
    ret


_farjmp:
    jmp far [esp+4]
    ret
    
_farcall:
    call far [esp+4]
    ret


_asm_hrb_api:
        STI
        PUSH    DS
        PUSH    ES
        PUSHAD        ; 保存のためのPUSH
        PUSHAD        ; hrb_apiにわたすためのPUSH
        MOV        AX,SS
        MOV        DS,AX        ; OS用のセグメントをDSとESにも入れる
        MOV        ES,AX
        CALL    _hrb_api
        CMP        EAX,0        ; EAXが0でなければアプリ終了処理
        JNE        end_app
        ADD        ESP,32
        POPAD
        POP        ES
        POP        DS
        IRETD
end_app:
;    EAXはtss.esp0の番地
        MOV        ESP,[EAX]
        POPAD
        RET            


; 从操作系统调用到用户程序
_start_app:        ; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
        PUSHAD        ; 32ビットレジスタを全部保存しておく
        MOV        EAX,[ESP+36]    ; アプリ用のEIP
        MOV        ECX,[ESP+40]    ; アプリ用のCS
        MOV        EDX,[ESP+44]    ; アプリ用のESP
        MOV        EBX,[ESP+48]    ; アプリ用のDS/SS
        MOV        EBP,[ESP+52]    ; tss.esp0の番地
        MOV        [EBP  ],ESP        ; OS用のESPを保存
        MOV        [EBP+4],SS        ; OS用のSSを保存
        MOV        ES,BX
        MOV        DS,BX
        MOV        FS,BX
        MOV        GS,BX
;    以下はRETFでアプリに行かせるためのスタック調整
        OR        ECX,3            ; アプリ用のセグメント番号に3をORする
        OR        EBX,3            ; アプリ用のセグメント番号に3をORする
        PUSH    EBX                ; アプリのSS
        PUSH    EDX                ; アプリのESP
        PUSH    ECX                ; アプリのCS
        PUSH    EAX                ; アプリのEIP
        RETF
