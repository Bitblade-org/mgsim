#include "tests.h"
#include <svp/abort.h>

void testQuickPageAlignedRW (result_t *res, char abort, char quiet){
	svp_abort();
	write64(S_SANDBOXW, 0x12345, quiet);
	addSResult(res, read64 (S_SANDBOXR, 0x12345, abort, quiet));
	addSResult(res, read64 (S0_SANDBOXR, 0x12345, abort, quiet));
	addSResult(res, read64 (S0_SANDBOXRW, 0x12345, abort, quiet));
	write64(S0_SANDBOXRW, 0x23456, quiet);
	addSResult(res, read64 (S_SANDBOXR, 0x23456, abort, quiet));
}
