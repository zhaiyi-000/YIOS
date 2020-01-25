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
	mov ah,2
	mov al,1
	mov ch,0
	mov cl,2
	mov dh,0
	mov dl,0
	mov bx,0
	int 0x13
	jc error

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