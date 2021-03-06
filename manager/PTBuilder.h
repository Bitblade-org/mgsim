#ifndef MANAGER_PTBUILDER_H_
#define MANAGER_PTBUILDER_H_

#include <stdio.h> //For printf MLDTODO Remove after debugging
#include <stdint.h>
#include <string.h>

#include "pagetable.h"
#include "pt_index.h"

//MLDOPT After adding a page to the PT, the manager will likely get a query for that page very quickly. Why not
//		 have the manager add the page to the PT, so the information is still in the D$?

int init_pt_set(pt_t* pt0, unsigned context_id, void *v_base, pt_t** next_table, size_t* free);
pt_t* construct_pt(pt_t** next_table);
int write_entry(pt_t* pt0, uint64_t pts_index, void* dst, unsigned lvl_offset, pt_t** next_table, size_t *free, uint8_t r, uint8_t w, uint8_t x);

#endif /* MANAGER_PTBUILDER_H_ */
