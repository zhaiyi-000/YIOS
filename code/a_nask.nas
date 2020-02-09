[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

    GLOBAL _api_putchar,_api_end,_api_putstr0,_api_openwin
    
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
