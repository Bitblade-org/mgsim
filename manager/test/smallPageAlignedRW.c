#include "tests.h"

void testSmallPageAlignedRW (result_t *res, char abort, char quiet){
	addSResult(res, read64 (0x500000, 0x12345, abort, quiet));
	addSResult(res, read64 (0x510000, 0x12345, abort, quiet));
	addSResult(res, read64 (0x520000, 0x12345, abort, quiet));
	write64(0x500000, 0x23456, quiet);
	addSResult(res, read64 (0x500000, 0x23456, abort, quiet));
	write64(0x530000, 0x34567, quiet);
	addSResult(res, read64 (0x500000, 0x34567, abort, quiet));
}
