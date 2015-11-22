#include "tests.h"

void testMediumPageBoundariesRW(result_t *result, char abort, char quiet){
	uint64_t mod;
	uint64_t baseR = M_SANDBOXR;
	uint64_t baseW = M_SANDBOXW;
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

	mod = MEDIUM_PAGE_SIZE - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = MEDIUM_PAGE_SIZE;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (MEDIUM_PAGE_SIZE * 2) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (MEDIUM_PAGE_SIZE * 2);
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (MEDIUM_PAGE_SIZE * 3) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));
}
