
[INSTRSET "i486p"]

VBEMODE    EQU        0x105

BOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

	
CYLS equ 0xff0
LEDS equ 0xff1
VMODE equ 0xff2
SCRNX equ 0xff4
SCRNY equ 0xff6
VRAM equ 0xff8

	org 0xc200
	
        MOV        AX,0x9000
        MOV        ES,AX
        MOV        DI,0
        MOV        AX,0x4f00
        INT        0x10
        CMP        AX,0x004f
        JNE        scrn320

; VBEのバージョンチェック

        MOV        AX,[ES:DI+4]
        CMP        AX,0x0200
        JB        scrn320            ; if (AX < 0x0200) goto scrn320

; 画面モード情報を得る

        MOV        CX,VBEMODE
        MOV        AX,0x4f01
        INT        0x10
        CMP        AX,0x004f
        JNE        scrn320

; 画面モード情報の確認

        CMP        BYTE [ES:DI+0x19],8
        JNE        scrn320
        CMP        BYTE [ES:DI+0x1b],4
        JNE        scrn320
        MOV        AX,[ES:DI+0x00]
        AND        AX,0x0080
        JZ        scrn320            ; モード属性のbit7が0だったのであきらめる

; 画面モードの切り替え

        MOV        BX,VBEMODE+0x4000
        MOV        AX,0x4f02
        INT        0x10
        MOV        BYTE [VMODE],8    ; 画面モードをメモする（C言語が参照する）
        MOV        AX,[ES:DI+0x12]
        MOV        [SCRNX],AX
        MOV        AX,[ES:DI+0x14]
        MOV        [SCRNY],AX
        MOV        EAX,[ES:DI+0x28]
        MOV        [VRAM],EAX
        JMP        keystatus

scrn320:
        MOV        AL,0x13            ; VGAグラフィックス、320x200x8bitカラー
        MOV        AH,0x00
        INT        0x10
        MOV        BYTE [VMODE],8    ; 画面モードをメモする（C言語が参照する）
        MOV        WORD [SCRNX],320
        MOV        WORD [SCRNY],200
        MOV        DWORD [VRAM],0x000a0000

; キーボードのLED状態をBIOSに教えてもらう

keystatus:

	mov ah,2
	int 0x16
	mov [LEDS],al
 
    
    mov al,0xff
    out 0x21,al  ;PIC0_IMR
    nop
    out 0xa1,al  ;PIC1_IMR
    cli
    
    call waitkbdout
    mov al,0xd1
    out 0x64,al
    call waitkbdout
    mov al,0xdf
    out 0x60,al
    call waitkbdout
    
[INSTRSET "i486p"]
    lgdt [gdtr0]
    mov eax,cr0
    and eax,0x7fffffff
    or eax,0x1
    mov cr0,eax
    jmp pipelineflush  ;我推测应该是流水段的原因
    
pipelineflush:
    mov ax,8
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax
    
    
    ;拷贝数据
    mov esi,bootpack
    mov edi,0x280000
    mov ecx,512*1024/4
    call memcpy
    
    
    mov esi,0x7c00
    mov edi,0x100000
    mov ecx,512/4
    call memcpy
    
    mov esi,0x8200
    mov edi,0x100000+0x200
    mov ecx,0
    mov cl, byte [CYLS]
    imul ecx,512*18*2/4
    sub ecx,512/4
    call memcpy
    
    
    mov ebx,0x280000
    mov ecx,[ebx+16]
    add ecx,3
    shr ecx,2
    jz skip
    mov esi,[ebx+20]
    add esi,ebx
    mov edi,[ebx+12]
    call memcpy
    
skip:
    mov esp,[ebx+12]
    jmp dword 16:0x1b
    
    
    
waitkbdout:
    in al,0x64
    and al,0x02
    in al,0x60  ;p167
    jnz waitkbdout
    ret

memcpy:
    mov eax,[esi]
    add esi,4
    mov [edi],eax
    add edi,4
    sub ecx,1
    jnz memcpy
    ret

    alignb 16
gdt0:
    resb 8
    dw 0xffff, 0x0000,0x9200,0x00cf
    dw 0xffff, 0x0000,0x9a28,0x0047
    
    dw 0  ;不知道干什么的

gdtr0:
    dw 3*8-1
    dd gdt0


    alignb 16
bootpack:
