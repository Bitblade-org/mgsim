#ifndef ARCH_DEV_JTAG_PAGETABLETYPES_H_
#define ARCH_DEV_JTAG_PAGETABLETYPES_H_

#include <cstdint>
#include <arch/Address.h>


namespace Simulator{

/* MLDTODO-DOC PID width choice
   	   Linux legacy (32-bit) used a PID up to 32768 (15 bits?).
   	   Now, any value up to 2^22 is allowed.
   	   http://www.tin.org/bin/man.cgi?section=5&topic=proc

   	   32.768*64/8= 212  KiB
   	   2^22 *64/8= 33,5 MiB

   	   For now, I will start with 2 levels for PID, 3 levels for pAddr.
   	   That means: 512 entries * 512 entries of PID (2^18 = 262.144 entries max)
*/

/* MLDTODO-DOC Page sizes
		Aligned entries?
			Since non-leaf entries point to the next page table, non-leaf entries need to
			contain the address of the next table. If we allow the tables to be placed
			anywhere, we would need to store the full address to the (first byte of the) table.
			On a 64-bit addressable system, this will take 64 bits, meaning we need to have entries
			larger than 64 bits.
			But, by aligning the address of the tables to a power of two, we do not have to store
			the last part (offset) of the address. To prevent holes, it's best to align to the
			bytesize of the tables.

		Entry size:
			As with tables, it is best to align entries to the size of the entry.

		Page table size:
			Alignment to the smallest page size, so we don't need to save extra bits.
 */




} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_PAGETABLETYPES_H_ */
