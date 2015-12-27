#ifndef ARCH_DRISC_MMU_ITLB_H_
#define ARCH_DRISC_MMU_ITLB_H_

#include "TLB.h"

namespace Simulator {
namespace drisc {
namespace mmu {

class iTLB: public TLB{
public:
    iTLB(const std::string& name, Object& parent, IOMessageInterface& ioif) : TLB(name, parent, ioif){};

	void GetDeviceIdentity(IODeviceIdentification& id) const
	{
	    if (!DeviceDatabase::GetDatabase().FindDeviceByName("MGSim", "iTLB", id))
	    {
	    	DeviceDatabase::GetDatabase().Print(std::cout);
	        throw InvalidArgumentException(*this, "Device identity not registered");
	    }
	}

};

}
}
}


#endif /* ARCH_DRISC_MMU_ITLB_H_ */
