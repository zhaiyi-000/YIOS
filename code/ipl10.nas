CYLS equ 10

	org 0x7c00

; 以下は標準的なFAT12フォーマットフロッピーディスクのための記述
		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; ブートセクタの名前を自由に書いてよい（8バイト）
		DW		512				; 1セクタの大きさ（512にしなければいけない）
		DB		1				; クラスタの大きさ（1セクタにしなければいけない）
		DW		1				; FATがどこから始まるか（普通は1セクタ目からにする）
		DB		2				; FATの個数（2にしなければいけない）
		DW		224				; ルートディレクトリ領域の大きさ（普通は224エントリにする）
		DW		2880			; このドライブの大きさ（2880セクタにしなければいけない）
		DB		0xf0			; メディアのタイプ（0xf0にしなければいけない）
		DW		9				; FAT領域の長さ（9セクタにしなければいけない）
		DW		18				; 1トラックにいくつのセクタがあるか（18にしなければいけない）
		DW		2				; ヘッドの数（2にしなければいけない）
		DD		0				; パーティションを使ってないのでここは必ず0
		DD		2880			; このドライブ大きさをもう一度書く
		DB		0,0,0x29		; よくわからないけどこの値にしておくといいらしい
		DD		0xffffffff		; たぶんボリュームシリアル番号
		DB		"HARIBOTEOS "	; ディスクの名前（11バイト）
		DB		"FAT12   "		; フォーマットの名前（8バイト）
		RESB	18				; とりあえず18バイトあけておく

entry:
	mov ax,0
	mov ss,ax
	mov sp,0x7c00
	mov ds,ax

	mov ax,0x0820
	mov es,ax
	mov ch,0
	mov cl,2
	mov dh,0

readloop:
	mov si,0
retry:
	mov dl,0
	mov bx,0
	mov al,1
	mov ah,2
	int 0x13
	jnc next
	add si,1
	cmp si,5
	jae error
	mov ah,0
	mov dl,0
	int 0x13
	jmp retry
next:
	mov ax,es
	add ax,0x20
	mov es,ax
	add cl,1
	cmp cl,18
	jbe readloop
	mov cl,1
	add dh,1
	cmp dh,2
	jb readloop
	mov dh,0
	add ch,1
	cmp ch,CYLS
	jb readloop

	mov [0xff0],ch
	jmp 0xc200

fin:
	hlt
	jmp fin



; show msg
error:
	mov si,msg
putloop:
	mov al,[si]
	add si,1
	cmp al,0
	je fin
	mov ah,0x0e
	mov bx,15
	int 0x10
	jmp putloop


msg:
	db 0x0a,0x0a
	db "hello zy0000"
	db 0x0a
	db 0

	resb 0x7dfe-$
	db 0x55,0xaa