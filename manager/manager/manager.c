#include "manager.h"

#include <svp/abort.h>

sl_def(manager_handle,, sl_shparm(pt_t*, pt0), sl_shparm(MgtMsg_t*, msg), sl_shparm(int, result)){
	pt_t* 		pt0 	= sl_getp(pt0);
	MgtMsg_t*	mgtMsg  = sl_getp(msg);

	sl_setp(result, handleMsg(pt0, mgtMsg));

	//Make sure the compiler knows we know it's weakness
	sl_getp(result);
	sl_setp(pt0, pt0);
	sl_setp(msg, mgtMsg);
}
sl_enddef

int handleMsg(pt_t* pt0, MgtMsg_t* msg){ //MLDTODO Move out of exclusive
	if(msg->type == MISS){
		return handleMiss(pt0, msg);
	}
	if(msg->type == INV){
		return handleInvalidation(pt0, msg);
	}

	return (0 - 8) - msg->type;
}

int handleInvalidation(pt_t* pt0, MgtMsg_t* req){
	(void)(req);
	PRINT_STRING("handleInvalidation called. Not implemented...\n");
	//MLDTODO Implement!
	return 1;
}

int handleMiss(pt_t* pt0, MgtMsg_t* msg){
	PRINT_STRING("\nHandling miss for context:");
	PRINT_UINT(msg->mReq.contextId);
	PRINT_STRING(", addr:");
	PRINT_HEX(msg->mReq.vAddr);
	PRINT_STRING(" (");
	PRINT_HEX(msg->mReq.vAddr << VADDR_LSO);
	PRINT_STRING("), lineref:");
	PRINT_UINT(msg->mReq.lineIndex);
	PRINT_CHAR('\n');
	PRINT_STRING("Using pagetable start ptr: ");
	PRINT_HEX(pt0);
	PRINT_CHAR('\n');



//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//	000000000 000000000 000000000 000000000 001111111 111111111 PROCID
	uint64_t addr = msg->mReq.vAddr;


	addr |= ((uint64_t)msg->mReq.contextId) << (VADDR_WIDTH - VADDR_LSO);
//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//  001111111 111111111 000000000 000000000 000000000 000000000 PROCID
//  001111111 111111111 111111111 111111111 000000000 000000000 |=

	pte_t* entry;
	unsigned levels;
	int result = walkPageTable(pt0, addr, (VADDR_WIDTH - VADDR_LSO) + CONTEXTID_WIDTH, &entry, &levels);
	PRINT_STRING("Walk result: (");
	PRINT_INT(result);
	PRINT_STRING(") Levels: ");
	PRINT_UINT(levels);
	PRINT_STRING(", Address:");
	PRINT_HEX(get_pointer(entry));
	PRINT_CHAR('\n');

	if(result < 0){ return result; }

	uint16_t caller = msg->mReq.caller;
	msg->type = REFILL;

	if(result){
		assert(((int)levels - TABLE_OFFSET) >= 0);
		msg->refill.table = levels - TABLE_OFFSET;
		msg->refill.pAddr = entry->addr;
		if(msg->mReq.tlbType == ITLB){
			msg->iRefill.execute = entry->page_p.x;
		}else{
			msg->dRefill.read = entry->page_p.r;
			msg->dRefill.write = entry->page_p.w;
		}
		msg->refill.present = 1;
	}else{
		msg->refill.present = 0;
	}

	return send_net_msg(msg, TRANSMIT_ADDR(caller));
}

/*
 * Expects to receive addresses without lsb offset bits.
 * Returns a pointer to the entry.
 * use getPointer(entry) to get the full address from the entry.
 */
int walkPageTable(pt_t* pt0, uint64_t addr, size_t len, pte_t** entry, unsigned* levels){
	print_pt_index(addr);

	pt_t* t = pt0;
	*levels = 0;

	while(len >= PT_INDEX_WIDTH){
		PRINT_STRING("len ");
		PRINT_UINT(len);
		PRINT_STRING("--walkPageTable arrived at level ");
		PRINT_UINT(*levels);
		PRINT_STRING(" with entries: (only present)\n");
		printTable(t);
		uint64_t offset = PT_INDEX_MASK & (addr >> (len - PT_INDEX_WIDTH));

		PRINT_STRING("Entry ID: ");
		PRINT_UINT(offset);
		PRINT_CHAR('\n');

		*entry = &(t->entries[offset]);
		len -= PT_INDEX_WIDTH;
		(*levels)++;

		if((*entry)->p != 1){ return 0; }
		if((*entry)->t != 1){ return 1; }
		else{ t = (pt_t*)get_pointer(*entry); }
	}
	return -16;
}

void printTable(pt_t* t){
	for(int i=0; i<512; i++){
		if(t->entries[i].p){
			PRINT_STRING("Entry ");
			PRINT_UINT(i);
			PRINT_STRING(": p=");
			PRINT_UINT(t->entries[i].p);
			PRINT_STRING(", t=");
			PRINT_UINT(t->entries[i].t);
			PRINT_CHAR('\n');
		}
	}
}
