#include "tests.h"

void testNotLineAlignedR(result_t *result, char abort, char quiet){
	for(int i=0; i<LINE_SIZE; i++){
		if(read8(COUNT8R_START + i, i % 256, 0, 1)){
			addSResult(result, read8(COUNT8R_START + i, i % 256, abort, quiet));
		}else{
			addSResult(result, 0);
		}
	}
}
