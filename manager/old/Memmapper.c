#include <mgsim.h>
#include <mtconf.h>
#include <stdio.h>
#include <svp/sep.h>

unsigned mapPage(void* base);
unsigned mapLine(void* base);

int main(void) {
	for(int i=0; i<64; i++){
		mapLine(i);
	}

    return 0;
}

// Each line is 64 x 512 x 8 = 262144 bytes = 256 KiB
unsigned mapLine(uint64_t index){
	uint64_t len = 64 * 512 * 8;

	uint64_t base = index * len;
	uint64_t last = base + 64 * 512 * 8;

	printf("%p - %p | ");

	for(int i=0; i<64; i++){
		if(mapPage(base + i)){
			putchar('X');
		}else{
			putchar('.');
		}
	}

	putchar(' ');
	putchar('|');
}

// Each page is 512 x sizeof(uint64_t) = 512*8=4096 B = 4 KiB
unsigned mapPage(uint64_t index){
	unsigned used = 0;

	uint64_t* end = ((uint64_t*)base) + 511;

	for(uint64_t* ptr = base; ptr < end; ptr++){
		if(*ptr != 0){
			used++;
			printf("BASE %p USED: %p = %u\n", base, ptr, *ptr);
		}
	}

	printf("%p t/m %p: %u\n", base, end-1, used);
}
