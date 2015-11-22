#include "manager.h"

#include <svp/abort.h>


sl_def(manager_init,,sl_shparm(unsigned, channel)){
	volatile uint64_t* cPtr = &(mg_devinfo.channels[sl_getp(channel)]);

	//Tell cpu.io_if.nmux that we want to receive notifications for this channel
	*cPtr = 1;

	// To stop compiler from whining
	sl_setp(channel, sl_getp(channel));
}
sl_enddef;

sl_def(manager, , sl_shparm(unsigned, channel), sl_shparm(uint64_t, pt0)) {
    sl_index(i);

	first_pt((pt_t*)sl_getp(pt0));
    int res = manager_main(sl_getp(channel));
    PRINT_STRING("Manager failed with error code ");
    PRINT_INT(res);
    svp_abort();

	// To stop compiler from whining
    sl_setp(channel, sl_getp(channel));
    sl_setp(pt0, sl_getp(pt0));
}
sl_enddef;


int manager_main(unsigned channelNr){
	volatile uint64_t *channel = &(mg_devinfo.channels[channelNr]);
	//Channel already initialised by manager_init.

	MgtMsg_t msgBuffer;
	int result;

	while(1){
		result = receive_net_msg(&msgBuffer, channel);
		if(result == 0)	{ continue; }
		if(result < 0)  { return result; } //A whoopsie occurred

		result = handleMsg(&msgBuffer);
		if(result < 0)	{ return result; } //A whoopsie occurred
	}

}

int receive_net_msg(MgtMsg_t* const msg, volatile uint64_t* from){
	msg->data.part[1] = *from;
	msg->data.part[0] = *from;

	return 1;
}

int send_net_msg(MgtMsg_t* const msg, volatile uint64_t* dst){
	dst[1] = msg->data.part[1];
	dst[0] = msg->data.part[0];
	return 1;
}


pt_t* first_pt(pt_t* ptr){
	static pt_t* pointer;

	if(ptr != NULL){
		pointer = ptr;
	}

	return pointer;
}

int handleMsg(MgtMsg_t* msg){
	if(msg->type == MISS){
		return handleMiss(msg);
	}
	if(msg->type == INV){
		return handleInvalidation(msg);
	}
	if(msg->type == SET && msg->set.property == SET_PT_ON_MGT){
		return handleSetPT(msg);
	}

	return (0 - 8) - msg->type;
}

int handleSetPT(MgtMsg_t* msg){
	PRINT_STRING("Setting PT pointer from ");
	PRINT_HEX(get_pointer(NULL));
	PRINT_STRING(" to ");
	PRINT_HEX((void*)msg->data.part[2]);
	PRINT_CHAR('\n');

	get_pointer((pte_t*)msg->set.val0);
	return 1;
}

int handleInvalidation(MgtMsg_t* req){
	(void)(req);
	PRINT_STRING("handleInvalidation called. Not implemented...\n");
	//MLDTODO Implement!
	return 1;
}

int handleMiss(MgtMsg_t* msg){
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
	PRINT_HEX(first_pt(NULL));
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
	int result = walkPageTable(addr, (VADDR_WIDTH - VADDR_LSO) + CONTEXTID_WIDTH, &entry, &levels);
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
int walkPageTable(uint64_t addr, size_t len, pte_t** entry, unsigned* levels){
	print_pt_index(addr);

	pt_t* t = first_pt(NULL);
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
