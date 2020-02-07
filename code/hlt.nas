[BITS 32]

    mov al,'A'
    call 2*8:0x00000149
fin:
    hlt
    jmp fin
