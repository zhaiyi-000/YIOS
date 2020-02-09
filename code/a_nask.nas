[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

    GLOBAL _api_putchar,_api_end,_api_putstr0,_api_openwin,_api_putstrwin,_api_boxfilwin
    
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
