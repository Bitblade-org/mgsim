/*
 * defines.h
 *
 *  Created on: 18 Jul 2015
 *      Author: nijntje
 */

#ifndef MANAGER_DEFINES_H_
#define MANAGER_DEFINES_H_

#include <stdint.h>

#define TRANSMIT_ADDR(ioDevId) mg_devinfo.base_addrs[ioDevId]

//#define PT_INDEX_WIDTH 9
//#define PT_LSB_OFFSET 12
//#define CONTEXTID_WIDTH 18 //Rounded up to a multiple of PT_OFFSET_WIDTH

#define TABLE_OFFSET 4 //MLDTODO Figure out what the heck I use this for

//           INDEX     OFFSET
//  PT LVL | BITS    | BITS     | ADDRESSES
// ========================================================
//  -0 | 6 |  0 +  0 | 0  (+12) | 4096       (4 KiB)
//  -1 | 5 |  9 +  0 | 9  (+12) | 2097152    (2 MiB)
//  -2 | 4 | 18 +  0 | 18 (+12) | 1073741824 (1 GiB)
//  -3 | 3 | 27 +  0 | 27 (+12) | (512 GiB)
//  -4 | 2 | 36 +  0 | 36 (+12) | (256 TiB)
//  -5 | 1 | 36 +  9 | 36 (+12) | 4096 contexts (C) + (256 TiB)
//  -6 | 0 | 36 + 18 | 36 (+12) | 2 MiC + (256 TiB)
//                     45
//                     54
//                     63

//MLDNOTE The initial reserved page size must be a multitude of sizeof(pt_t)
//MLDNOTE sizeof(pt_t) must equal 2^VADDR_LSO
//MLDNOTE sizeof(void*) must be =< uint64_t
#define CONTEXTID_WIDTH 18  //Rounded up to a multiple of PT_OFFSET_WIDTH
#define VADDR_WIDTH 48
#define VADDR_LSO 12 // Least significant offset
#define PT_INDEX_WIDTH 9
//#define PTS_NR_LVL 6
#define INITIAL_RESERVE_LVL_OFFSET 1

//Calculated:
//#define PTS_INDEX_WIDTH PT_INDEX_WIDTH * PTS_NR_LVL
#define PTS_INDEX_WIDTH (VADDR_WIDTH + CONTEXTID_WIDTH - VADDR_LSO)
#define PT_INDEX_MASK (~(UINT64_MAX << PT_INDEX_WIDTH))

//VADDR_SIGNIFICANT_WIDTH + PROCID_WIDTH = 54 //Must be a multiple of PT_OFFSET_WIDTH


#endif /* MANAGER_DEFINES_H_ */
