void io_hlt();
void write_mem8(int addr, int data);

void HariMain(){
	int i;

	for(i = 0xa0000;i < 0xb0000;i++){
		write_mem8(i,15);
	}

	for(;;){
		io_hlt();
	}
}