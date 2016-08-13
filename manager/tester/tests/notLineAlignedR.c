#include "tests.h"

sl_def(testNotLineAlignedR,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	for(int i=0; i<LINE_SIZE; i++){
		if(read8(COUNT8R_START + i, i % 256, 0, 1)){
			addSResult(result, read8(COUNT8R_START + i, i % 256, abort, quiet));
		}else{
			addSResult(result, 0);
		}
	}

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
