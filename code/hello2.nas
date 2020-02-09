;[INSTRSET "i486p"]
;[BITS 32]
;        MOV        EDX,2
;        MOV        EBX,msg
;        INT        0x40
;        mov edx,4
;        int 0x40
;msg:
;        DB    "hello",0
;

;hello5.nas

[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello5.nas"]

        GLOBAL    _HariMain

[SECTION .text]

_HariMain:
        MOV        EDX,2
        MOV        EBX,msg
        INT        0x40
        MOV        EDX,4
        INT        0x40

[SECTION .data]

msg:
        DB    "hello, world", 0x0a, 0

