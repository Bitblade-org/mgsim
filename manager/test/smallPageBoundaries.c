#include "tests.h"

void testSmallPageBoundariesR(result_t *result, char abort, char quiet){
	uint64_t baseR = COUNT64R_START;
	uint64_t mod;
	uint64_t data;

	mod = 0;
	data = 0;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2) - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2);
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 3) - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));
}

void testSmallPageBoundariesRW(result_t *result, char abort, char quiet){
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

	mod = SMALL_PAGE_SIZE - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2);
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 3) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));
}
