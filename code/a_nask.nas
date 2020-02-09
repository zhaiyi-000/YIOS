[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

    GLOBAL _api_putchar,_api_end,_api_putstr0
    
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
