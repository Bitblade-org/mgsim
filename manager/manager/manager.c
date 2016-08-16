#include "manager.h"

#include <svp/abort.h>

sl_def(handle_miss,, sl_shparm(pt_t*, pt0), sl_shparm(MgtMsg_t*, msg)){
	pt_t* 		pt0 	= sl_getp(pt0);
	MgtMsg_t*	mgtMsg  = sl_getp(msg);

	uint64_t addr = mgtMsg->mReq.vAddr;

	addr |= ((uint64_t)mgtMsg->mReq.contextId) << (VADDR_WIDTH - VADDR_LSO);

	pte_t* entry;
	int result = walkPageTable(pt0, addr, &entry);

	uint16_t caller = mgtMsg->mReq.caller;
	mgtMsg->type = REFILL;

	if(result){
		mgtMsg->refill.table = result - TABLE_OFFSET;
		mgtMsg->refill.pAddr = entry->addr;
		if(mgtMsg->mReq.tlbType == ITLB){
			mgtMsg->iRefill.execute = entry->page_p.x;
		}else{
			mgtMsg->dRefill.read = entry->page_p.r;
			mgtMsg->dRefill.write = entry->page_p.w;
		}
		mgtMsg->refill.present = 1;
	}else{
		mgtMsg->refill.present = 0;
	}

	send_net_msg(mgtMsg, TRANSMIT_ADDR(caller));

	//Make sure the compiler knows we know it's weakness
	sl_setp(pt0, pt0);
	sl_setp(msg, mgtMsg);
}
sl_enddef

/*
 * Expects to receive addresses without lsb offset bits.
 * Returns a pointer to the entry.
 * use getPointer(entry) to get the full address from the entry.
 */
inline int walkPageTable(pt_t* t, uint64_t addr, pte_t** entry){
	register size_t len = (VADDR_WIDTH - VADDR_LSO) + CONTEXTID_WIDTH;
	register int levels = 0;

	while(len >= PT_INDEX_WIDTH){
		uint64_t offset = PT_INDEX_MASK & (addr >> (len - PT_INDEX_WIDTH));

		*entry = &(t->entries[offset]);
		len -= PT_INDEX_WIDTH;
		levels++;

		if((*entry)->p != 1){ return 0; }
		if((*entry)->t != 1){ return levels; }
		else{
			t = (pt_t*)((uint64_t)((*entry)->addr << VADDR_LSO));
		}
	}

	//UNREACHABLE
	svp_abort();
}

//sl_def(handle_miss,, sl_shparm(pt_t*, pt0), sl_shparm(MgtMsg_t*, msg)){
//	pt_t* 		t 	= sl_getp(pt0);
//	MgtMsg_t*	mgtMsg  = sl_getp(msg);
//
//	uint64_t addr = mgtMsg->mReq.vAddr;
//
//	addr |= ((uint64_t)mgtMsg->mReq.contextId) << (VADDR_WIDTH - VADDR_LSO);
//
//	uint16_t caller = mgtMsg->mReq.caller;
//	mgtMsg->type = REFILL;
//
//	pte_t* entry;
//	unsigned int levels = 0;
//
//	size_t len = VADDR_WIDTH - VADDR_LSO + CONTEXTID_WIDTH;
//
//	while(len >= PT_INDEX_WIDTH){
//		uint64_t offset = PT_INDEX_MASK & (addr >> (len - PT_INDEX_WIDTH));
//
//		entry = &(t->entries[offset]);
//		len -= PT_INDEX_WIDTH;
//		(levels)++;
//
//		if(entry->p != 1){
//			mgtMsg->refill.present = 0;
//			send_net_msg(mgtMsg, TRANSMIT_ADDR(caller));
//			break;
//		}
//		if(entry->t != 1){
//			mgtMsg->refill.table = levels - TABLE_OFFSET;
//			mgtMsg->refill.pAddr = entry->addr;
//			if(mgtMsg->mReq.tlbType == ITLB){
//				mgtMsg->iRefill.execute = entry->page_p.x;
//			}else{
//				mgtMsg->dRefill.read = entry->page_p.r;
//				mgtMsg->dRefill.write = entry->page_p.w;
//			}
//			mgtMsg->refill.present = 1;
//			send_net_msg(mgtMsg, TRANSMIT_ADDR(caller));
//			break;
//		}
//		else{
//			t = (pt_t*)((uint64_t)(entry->addr << VADDR_LSO));
//		}
//	}
//
//	//Make sure the compiler knows we know it's weakness
//	sl_setp(pt0, 0);
//	sl_setp(msg, 0);
//}
//sl_enddef
//
///*
// * Expects to receive addresses without lsb offset bits.
// * Returns a pointer to the entry.
// * use getPointer(entry) to get the full address from the entry.
// */
//inline int walkPageTable(pt_t* pt0, uint64_t addr, size_t len, pte_t** entry, unsigned* levels){
//
//
//	//UNREACHABLE
//	svp_abort();
//}
