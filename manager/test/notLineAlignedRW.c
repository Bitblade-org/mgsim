#include "tests.h"

void testNotLineAlignedRW(result_t *result, char abort, char quiet){
	write8(SANDBOXW_START, 0xFF, 0);
	write8(SANDBOXW_START + 1, 0x55, 0);
	addSResult(result, read8(SANDBOXR_START, 0xFF, abort, quiet));
	addSResult(result, read8(SANDBOXR_START + 1, 0x55, abort, quiet));

	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<128; testNr++){
		write8(SANDBOXW_START + testNr, testNr, 1);
		if(read8(SANDBOXR_START + testNr, testNr, 0, 1)){
			fails++;
			write8(SANDBOXW_START + testNr, testNr, quiet);
			read8(SANDBOXR_START + testNr, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due to many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);
}

