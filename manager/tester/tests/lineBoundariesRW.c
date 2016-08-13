#include <stdlib.h>

#include "tests.h"

sl_def(testLineBoundariesRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t mod;
	uint64_t baseR = S_SANDBOXR;
	uint64_t baseW = S_SANDBOXW;
	uint64_t data;
	//Cannot use random numbers according to SL17:
	// 	However, the following library features are not supported on the MGSim targets:
	//		...
	//		random number generation (oversight; expect support soon)
	//		...

	mod = 0;
	data = 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = LINE_SIZE - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = LINE_SIZE;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (LINE_SIZE * 2) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (LINE_SIZE * 2);
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (LINE_SIZE * 3) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
