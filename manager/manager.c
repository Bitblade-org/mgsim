#include "manager.h"

//MLDTODO Remove all printf after testing

reqQueue_t* getQueue(){
	static uint8_t initialised = 0;
	static reqQueue_t q;
	int i = 0;

	if(!initialised){
		q.head = q.tail = 0;
		for(i=0; i<R_QUEUE_SIZE; i++){
			q.buffer[i].base.free = 1;
		}
		initialised = 1;
	}

	return &q;
}

pt_t* firstPt(pt_t* ptr){
	static pt_t* pointer;

	if(ptr != NULL){
		pointer = ptr;
	}

	return pointer;
}


int in_netMsg(managerReq_t* const msg){
	reqQueue_t* q = getQueue();

	return push(q, msg);
}

int handleReq(){
	reqQueue_t* q = getQueue();
	managerReq_t* req;
	if(!peek(q, &req)){
		return 0;
	}

	int result;
	if(req->base.isInvalidate){
		result = handleInvalidation(req);
	}else{
		result = handleMiss(req);
	}

	if(result > 0){
		if(!pop(q)){
			return -20;
		}
	}

	return result;
}

int handleInvalidation(managerReq_t* req){
	(void)(req);
	printf("handleInvalidation called. Not implemented...\n");
	//MLDTODO Implement!
	return 1;
}

//MLDOPT Store result if outgoing buffer is full
int handleMiss(managerReq_t* req){
	printf("\nHandling miss for proc:%u, addr:%lu\n", req->miss.processId, req->miss.vAddr);
	printf("Using pagetable start ptr: %p\n", firstPt(NULL));


//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//	000000000 000000000 000000000 000000000 001111111 111111111 PROCID
	uint64_t addr = req->miss.vAddr;


	addr |= ((uint64_t)req->miss.processId) << VADDR_SIGNIFICANT_WIDTH;
//  000000000 000000000 111111111 111111111 000000000 000000000 ADDR
//  001111111 111111111 000000000 000000000 000000000 000000000 PROCID
//  001111111 111111111 111111111 111111111 000000000 000000000 |=

	pte_t* entry;
	unsigned levels;
	int result = walkPageTable(addr, VADDR_SIGNIFICANT_WIDTH + PROCID_WIDTH, &entry, &levels);
	printf("Walk result: (%d) Levels: %d, Address:%p\n", result, levels, getPointer(entry));

	if(result < 0){ return result; }

	tlbRefillMsg response;
	memset(&response, 0, sizeof(tlbRefillMsg)); //MLDTODO Remove after testing

	response.base.dTLB = req->miss.tlbType;
	response.base.lineIndex = req->miss.lineIndex;
	response.base.present = result;

	if(result){
		assert(((int)levels - TABLE_OFFSET) >= 0);
		response.base.table = levels - TABLE_OFFSET;
		response.base.pAddr = entry->base.addr;
		if(req->miss.tlbType == 0){
			response.i.execute = entry->page_p.x;
		}else{
			response.d.read = entry->page_p.r;
			response.d.write = entry->page_p.w;
		}
	}

	out_RefillMessage(&response);
	//MLDTODO Implement!
	return 1;
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
	return -1;
}

void printTable(pt_t* t){
	for(int i=0; i<512; i++){
		if(t->entries[i].base.p){
			printf("Entry %d: p=%u, t=%u\n", i, t->entries[i].base.p, t->entries[i].base.t);
		}
	}
}


void loop(void){
	int result;
	do{
		result = handleReq();
	}while(result >= 0);

	printf("ERROR: Return code %d\n", result);
	assert(0);
}


