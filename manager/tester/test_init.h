#ifndef MANAGER_MEMTESTER_H_
#define MANAGER_MEMTESTER_H_

#include <stddef.h>

#include "../ptLib/pagetable.h"

void tester_init(int contextId, void* ptsPBase, pt_t** next_table, size_t* free);
void createPageEntries(int contextId, void* ptsPBase, pt_t** next_table, size_t* free);
void writeStartingData();




#endif /* MANAGER_MEMTESTER_H_ */
