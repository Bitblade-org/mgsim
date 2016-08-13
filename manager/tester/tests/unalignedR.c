#include "tests.h"

sl_def(testUnalignedRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t data = 0;

	for(int i=0; i<8; i++){
		write64(S_SANDBOXW + i, data, quiet);
		addSResult(result, read64(S_SANDBOXR + i, data, abort, quiet));
		data = ~data;
	}

	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere on the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;
