#include "MMU.h"

#include <stddef.h>

#include "../../../sim/config.h"
#include "../../../sim/inputconfig.h"

namespace Simulator {
namespace drisc{
namespace mmu{

MMU::MMU(const std::string& name, Object& parent, AddrWidth netAddrWidth)
	: Object(name, parent),
	m_pAddrWidth(GetTopConf("PhysicalAddressWidth", size_t)),
	m_vAddrWidth(GetConf("VirtualAddressWidth", size_t)),
	m_procAddrWidth(GetConf("ProcessIdWidth", size_t)),
	m_netAddrWidth(netAddrWidth),
	m_dtlb("dtlb", *this),
	m_itlb("itlb", *this)
{}

void MMU::onInvalidateMsg(RemoteMessage &msg){
	m_dtlb.onInvalidateMsg(msg);
	m_itlb.onInvalidateMsg(msg);
}


void MMU::Cmd_Info(std::ostream& out, const std::vector<std::string>& /*arguments*/) const{
    out << "The MMU blablabla\n\n";
    out << "   Virtual address width: "	<< unsigned(getVAddrWidth()) 	<< " bits\n";
    out << "  Physical address width: "	<< unsigned(getPAddrWidth()) 	<< " bits\n";
    out << "        Process ID width: "	<< unsigned(getProcAddrWidth()) << " bits\n";
    out << "   Network address width: " << unsigned(getNetAddrWidth()) << " bits\n";
    out << "\n";
    out << "Supported operations:\n";
    out << "  None\n";
    out << std::flush;
}

//const TLB		m_dtlb;
//const TLB		m_itlb;
//const AddrWidth	m_pAddrWidth;
//const AddrWidth	m_vAddrWidth;
//const AddrWidth	m_procAddrWidth;
//const RAddr		m_pAddrTemplate;
//const RAddr		m_vAddrTemplate;
//const RAddr		m_procAddrTemplate;

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
