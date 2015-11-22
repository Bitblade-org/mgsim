#include "tests.h"

void testUnalignedRW(result_t *result, char abort, char quiet){
	uint64_t data = 0;

	for(int i=0; i<8; i++){
		write64(S_SANDBOXW + i, data, quiet);
		addSResult(result, read64(S_SANDBOXR + i, data, abort, quiet));
		data = ~data;
	}
}
