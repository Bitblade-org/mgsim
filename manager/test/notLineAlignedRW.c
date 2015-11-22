#include "tests.h"

void testNotLineAlignedRW(result_t *result, char abort, char quiet){
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
}

