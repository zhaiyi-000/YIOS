[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "naskfunc.nas"]
	GLOBAL _io_hlt,_write_mem8
[SECTION .text]

_io_hlt:
	hlt
	ret
