#include "MMU.h"

namespace Simulator {
namespace drisc{
namespace mmu{

MMU::MMU(const std::string& name, Object& parent)
	: Object(name, parent),
	m_dtlb("dtlb", *this),
	m_itlb("itlb", *this)
{}

void MMU::onInvalidateMsg(RemoteMessage &msg){
	m_dtlb.onInvalidateMsg(msg);
	m_itlb.onInvalidateMsg(msg);
}


void MMU::Cmd_Info(std::ostream& out, const std::vector<std::string>& /*arguments*/) const{
    out << "The MMU blablabla\n\n";
    out << "   Virtual address size: " 	<< unsigned(RAddr::VirtWidth) 	<< " bytes\n";
    out << "  Physical address size: " 	<< unsigned(RAddr::PhysWidth) 	<< " bytes\n";
    out << "        Process ID size: " 	<< unsigned(RAddr::ProcIdWidth) 	<< " bytes\n";
    out << "\n";
    out << "Supported operations:\n";
    out << "  None\n";
    out << std::flush;
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
