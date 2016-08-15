#include "../MgtMsg.h"
#include "../defines.h"

#include "tlbControl.h"

void sendSinglePartMessage(MgtMsg_t* message, tlbRef_t tlbReference);
void sendDualPartMessage(MgtMsg_t* message, tlbRef_t tlbReference);

void enableTlb(unsigned int managerIOid, unsigned int managerChannel, tlbRef_t tlbReference){

	MgtMsg_t message;
	message.type = SET;
	message.set.property = SET_MGT_ADDR_ON_TLB;
	message.set.val1 = managerIOid;
	message.set.val2 = managerChannel;
	sendSinglePartMessage(&message, tlbReference);

	// Our prayers be with those threads who remain on the core during the
	// imminent all-fubarring apocalypse.
	message.set.property = SET_STATE_ON_TLB;
	message.set.val1 = 1;
	sendSinglePartMessage(&message, tlbReference);
}

void invalidateTlb(tlbRef_t tlbReference){
	//MLDTODO-DOC TLB can be controlled through messages!!!

	MgtMsg_t message;
	message.type = INVALIDATE;
	message.iReq.caller = getIOAddr();
	message.iReq.contextId = 0;
	message.iReq.vAddr = 0;
	message.iReq.filterContext = 0;
	message.iReq.filterVAddr = 0;

	sendDualPartMessage(&message, tlbReference);

	for(int i=0; i<200; i++){
		asm volatile ("nop");
	}
}

tlbRef_t getTlbReference(struct mg_io_info* ioInfo, size_t core_id, char tlbType){
	struct mg_device_id search;
	search.provider = 1;
	search.model = 10 + tlbType;
	search.revision = 1;

	tlbRef_t reference;
	reference.nocId = find_core_device(ioInfo, core_id, &search);
	return reference;
}

void sendSinglePartMessage(MgtMsg_t* message, tlbRef_t tlbReference){
	volatile uint64_t* mmioAddress = TRANSMIT_ADDR(tlbReference.nocId);

	// A single part message only contains data in the first 64 bits.
	// The TLB expects to receive a single-part message written to the 3rd word of its noc interface
	mmioAddress[2] = message->data.part[0];
}

void sendDualPartMessage(MgtMsg_t* message, tlbRef_t tlbReference){
	volatile uint64_t* mmioAddress = TRANSMIT_ADDR(tlbReference.nocId);

	// The TLB expects to receive a dual-part message written to the 1st and 2nd words of its noc interface.
	// The TLB starts interpreting the message as soon as word 1 is received, so we have to transmit this last.
	mmioAddress[1] = message->data.part[1];
	mmioAddress[0] = message->data.part[0];
}
