/*
 * defines.h
 *
 *  Created on: 18 Jul 2015
 *      Author: nijntje
 */

#ifndef MANAGER_DEFINES_H_
#define MANAGER_DEFINES_H_

#define PT_INDEX_WIDTH 9
#define PT_LSB_OFFSET 12
#define VADDR_SIGNIFICANT_WIDTH 36
#define PROCID_WIDTH 18 //Rounded up to a multiple of PT_OFFSET_WIDTH

#define TABLE_OFFSET 4


//VADDR_SIGNIFICANT_WIDTH + PROCID_WIDTH = 54 //Must be a multiple of PT_OFFSET_WIDTH


#endif /* MANAGER_DEFINES_H_ */
