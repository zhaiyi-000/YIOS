[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

    GLOBAL _api_putchar,_api_end,_api_putstr0,_api_openwin,_api_putstrwin,_api_boxfilwin
    GLOBAL _api_initmalloc,_api_malloc,_api_free,_api_point,_api_refreshwin,_api_linewin
    GLOBAL _api_closewin,_api_getkey,_api_alloctimer,_api_inittimer,_api_settimer,_api_freetimer
    GLOBAL _api_beep
    
[SECTION .text]

_api_putchar:
    mov edx,1
    mov al,[esp+4]
    int 0x40
    ret

_api_end:
    mov edx,4
    int 0x40
    
    
_api_putstr0:    ; void api_putstr0(char *s);
    PUSH    EBX
    MOV        EDX,2
    MOV        EBX,[ESP+8]        ; s
    INT        0x40
    POP        EBX
    RET

_api_openwin:   ;int api_openwin(char *buf, int xsiz, int ysiz, int clo_inv, char *title)

    push edi
    push esi
    push ebx
    mov edx,5
    mov ebx,[esp+16]
    mov esi,[esp+20]
    mov edi,[esp+24]
    mov eax,[esp+28]
    mov ecx,[esp+32]
    
    int 0x40
    pop ebx
    pop esi
    pop edi
    
    ret
    
_api_putstrwin:
    push edi
    push esi
    push ebp
    push ebx
    mov edx,6
    mov ebx,[esp+20]
    mov esi,[esp+24]
    mov edi,[esp+28]
    mov eax,[esp+32]
    mov ecx,[esp+36]
    mov ebp,[esp+40]
    
    int 0x40
    pop ebx
    pop ebp
    pop esi
    pop edi
    
    ret
    
_api_boxfilwin:
    push edi
    push esi
    push ebp
    push ebx
    mov edx,7
    mov ebx,[esp+20]
    mov eax,[esp+24]
    mov ecx,[esp+28]
    mov esi,[esp+32]
    mov edi,[esp+36]
    mov ebp,[esp+40]
    
    int 0x40
    pop ebx
    pop ebp
    pop esi
    pop edi

    ret


_api_initmalloc:  ;void api_initmalloc(void)

    push ebx
    mov edx,8
    mov ebx,[cs:0x20]
    mov eax,ebx
    add eax,32*1024   ;为memman申请的内存
    mov ecx,[cs:0x0]
    sub ecx,eax
    int 0x40
    pop ebx
    ret

_api_malloc:
    push ebx
    mov edx,9
    mov ebx,[cs:0x20]
    mov ecx,[esp+8]
    int 0x40
    pop ebx
    ret
    
_api_free:
    push ebx
    mov edx,10
    mov ebx,[cs:0x20]
    mov eax,[esP+8]
    mov ecx,[esp+12]
    int 0x40
    pop ebx
    ret
    
_api_point:
    push edi
    push esi
    push ebx
    mov edx,11
    mov ebx,[esp+16]
    mov esi,[esp+20]
    mov edi,[esp+24]
    mov eax,[esp+28]
    int 0x40
    pop ebx
    pop esi
    pop edi
    
    ret


_api_refreshwin:
    push edi
    push esi
    push ebx
    mov edx,12
    mov ebx,[esp+16]
    mov eax,[esp+20]
    mov ecx,[esp+24]
    mov esi,[esp+28]
    mov edi,[esp+32]
    int 0x40
    pop ebx
    pop esi
    pop edi
    
    ret

_api_linewin:
    push edi
    push esi
    push ebp
    push ebx
    mov edx,13
    mov ebx,[esp+20]
    mov eax,[esp+24]
    mov ecx,[esp+28]
    mov esi,[esp+32]
    mov edi,[esp+36]
    mov ebp,[esp+40]
    
    int 0x40
    
    pop ebx
    pop ebp
    pop esi
    pop edi
    
    ret
    
_api_closewin:
    push ebx
    mov edx,14
    mov ebx,[esp+8]
    int 0x40
    pop ebx
    
    ret

_api_getkey:
    mov edx,15
    mov eax,[esp+4]
    int 0x40
    
    ret


_api_alloctimer:    ; int api_alloctimer(void);
        MOV        EDX,16
        INT        0x40
        RET

_api_inittimer:        ; void api_inittimer(int timer, int data);
        PUSH    EBX
        MOV        EDX,17
        MOV        EBX,[ESP+ 8]        ; timer
        MOV        EAX,[ESP+12]        ; data
        INT        0x40
        POP        EBX
        RET

_api_settimer:        ; void api_settimer(int timer, int time);
        PUSH    EBX
        MOV        EDX,18
        MOV        EBX,[ESP+ 8]        ; timer
        MOV        EAX,[ESP+12]        ; time
        INT        0x40
        POP        EBX
        RET

_api_freetimer:        ; void api_freetimer(int timer);
        PUSH    EBX
        MOV        EDX,19
        MOV        EBX,[ESP+ 8]        ; timer
        INT        0x40
        POP        EBX
        RET

_api_beep:            ; void api_beep(int tone);
    MOV        EDX,20
    MOV        EAX,[ESP+4]            ; tone
    INT        0x40
    RET
