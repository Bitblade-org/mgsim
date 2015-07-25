#include "manager.h"

//MLDTODO Remove all printf after testing

int main(){
	volatile uint64_t *channel = &(mg_devinfo.channels[NOTIFICATION_CHANNEL]);

	MgtMsg_t msgBuffer;
	int result;

	//Tell cpu.io_if.nmux that we want to receive notifications for this channel
	*channel = 1;

	//MLDTODO Remove after testing
	//Inform JTAG about our existence
	volatile uint64_t* jtag = mg_devinfo.base_addrs[12];
	jtag[5] = NOTIFICATION_CHANNEL;

	while(1){
		printf("Manager ready to receive request\n");
		result = receive_net_msg(&msgBuffer, channel);
		if(result == 0)	{ continue; }
		if(result < 0)  { return result; } //A whoopsie occurred

//		do{
			printf("Manager handling request\n");
			result = handleMsg(&msgBuffer);
			if(result < 0)	{ return result; } //A whoopsie occurred
//		}while(result == 0);

//		do{
			printf("Manager sending response\n");
			result = send_net_msg(&msgBuffer, mg_devinfo.base_addrs[result]);
			if(result < 0)	{ return result; } //A whoopsie occurred
//		}while(result == 0);
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


pt_t* firstPt(pt_t* ptr){
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
	if(msg->type == INV_RQ){
		return handleInvalidation(msg);
	}
	if(msg->type == SET_PT){
		return handleSetPT(msg);
	}

	return (0 - 8) - msg->type;
}

int handleSetPT(MgtMsg_t* msg){
	printf("Setting PT pointer from %p to %p\n", getPointer(NULL), (void*)msg->data.part[2]);
	getPointer((pte_t*)msg->setPT.pointer);
	return 1;
}

int handleInvalidation(MgtMsg_t* req){
	(void)(req);
	printf("handleInvalidation called. Not implemented...\n");
	//MLDTODO Implement!
	return 1;
}

int handleMiss(MgtMsg_t* msg){
	printf("\nHandling miss for context:%u, addr:%lu\n", msg->mReq.contextId, msg->mReq.vAddr);
	printf("Using pagetable start ptr: %p\n", firstPt(NULL));


//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//	000000000 000000000 000000000 000000000 001111111 111111111 PROCID
	uint64_t addr = msg->mReq.vAddr;


	addr |= ((uint64_t)msg->mReq.contextId) << VADDR_SIGNIFICANT_WIDTH;
//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//  001111111 111111111 000000000 000000000 000000000 000000000 PROCID
//  001111111 111111111 111111111 111111111 000000000 000000000 |=

	pte_t* entry;
	unsigned levels;
	int result = walkPageTable(addr, VADDR_SIGNIFICANT_WIDTH + PROCID_WIDTH, &entry, &levels);
	printf("Walk result: (%d) Levels: %d, Address:%p\n", result, levels, getPointer(entry));

	if(result < 0){ return result; }

	uint16_t caller = msg->mReq.caller;
	if(result){
		assert(((int)levels - TABLE_OFFSET) >= 0);
		msg->type = REFILL;
		msg->refill.table = levels - TABLE_OFFSET;
		msg->refill.pAddr = entry->base.addr;
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

	return caller;
}

/*
 * Expects to receive addresses without lsb offset bits.
 * Returns a pointer to the entry.
 * use getPointer(entry) to get the full address from the entry.
 */
int walkPageTable(uint64_t addr, size_t len, pte_t** entry, unsigned* levels){
	printf("walkPageTable... Input address[%zd]: ", len);
	for(int i=len-1; i>=0; i--){
		int val = (addr >> i) & 1;
		if((i+1)%PT_INDEX_WIDTH == 0){
			printf(" ");
		}
		printf("%d", val);
	}
	printf("\n");

	static uint64_t mask = ~(UINT64_MAX << PT_INDEX_WIDTH);

	pt_t* t = firstPt(NULL);
	*levels = 0;

	while(len >= PT_INDEX_WIDTH){
		printf("walkPageTable arrived at level %u with entries: (only present)\n", *levels);
		printTable(t);
		uint64_t offset = mask & (addr >> (len - PT_INDEX_WIDTH));

		printf("Entry ID: %lu\n", offset);

		*entry = &(t->entries[offset]);
		len -= PT_INDEX_WIDTH;
		(*levels)++;

		if((*entry)->base.p != 1){ return 0; }
		if((*entry)->base.t != 1){ return 1; }
		else{ t = (pt_t*)getPointer(*entry); }
	}
	return -16;
}

void printTable(pt_t* t){
	for(int i=0; i<512; i++){
		if(t->entries[i].base.p){
			printf("Entry %d: p=%u, t=%u\n", i, t->entries[i].base.p, t->entries[i].base.t);
		}
	}
}
