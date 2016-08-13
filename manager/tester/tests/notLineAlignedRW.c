#include "tests.h"

sl_def(testNotLineAlignedRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	write8(S_SANDBOXW, 0xFF, quiet);
	addSResult(result, read8(S_SANDBOXR, 0xFF, abort, quiet));

	write8(S_SANDBOXW + 1, 0x55, quiet);
	addSResult(result, read8(S_SANDBOXR, 0xFF, abort, quiet));
	addSResult(result, read8(S_SANDBOXR + 1, 0x55, abort, quiet));

	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<(2*LINE_SIZE); testNr++){
		write8(S_SANDBOXW + testNr, testNr, 1);
		if(read8(S_SANDBOXR + testNr, testNr, 0, 1)){
			fails++;
			write8(S_SANDBOXW + testNr, testNr, quiet);
			read8(S_SANDBOXR + testNr, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;

