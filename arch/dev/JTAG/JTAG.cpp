/*
 * JTAG.cpp
 *
 *  Created on: 18 Jul 2015
 *      Author: nijntje
 */

#include "JTAG.h"

#include <iostream>

#include "../../../sim/kernel.h"


namespace Simulator {

JTAG::JTAG(const std::string& name, Object& parent, IOMessageInterface& iobus, IODeviceID devid):
		Object(name, parent)
{
	iobus.RegisterClient(devid, *this);
}

bool JTAG::OnWriteRequestReceived(IODeviceID /*from*/, MemAddr /*address*/, const IOData& /*data*/){
	return true;
}

} /* namespace Simulator */
