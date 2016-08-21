#ifndef MANAGER_TLBLIB_TLBCONTROL_H_
#define MANAGER_TLBLIB_TLBCONTROL_H_

#include "../ioLib/IO.h"
#include "tlbRef.h"

#define TLB_TYPE_I 0
#define TLB_TYPE_D 1

void enableTlb(unsigned int managerIOid, unsigned int managerChannel, tlbRef_t tlbReference);
//void invalidateTlb(tlbRef_t tlbReference);
tlbRef_t getTlbReference(struct mg_io_info* ioInfo, size_t core_id, char tlbType);


#endif /* MANAGER_TLBLIB_TLBCONTROL_H_ */
