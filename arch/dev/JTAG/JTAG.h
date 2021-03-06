#ifndef ARCH_DEV_JTAG_JTAG_H_
#define ARCH_DEV_JTAG_JTAG_H_

#include <string>

#include "../../../sim/kernel.h"
#include "../../IOBus.h"
#include "../../simtypes.h"
#include <arch/IOMessageInterface.h>






namespace Simulator {

class JTAG: public Object, public IIOMessageClient {


public:
	JTAG(const std::string& name, Object& parent, IOMessageInterface& ioif, IODeviceID devid);

	bool OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data);
    const std::string& GetIODeviceName() const {return GetName();}
    void GetDeviceIdentity(IODeviceIdentification& id) const {id = IODeviceIdentification{0,0,0};}

};

} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_JTAG_H_ */
