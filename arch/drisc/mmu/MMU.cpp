#include "MMU.h"

namespace Simulator {
namespace drisc{
namespace mmu{

MMU::MMU(const std::string& name, Object& parent, Config& config)
	: Object(name, parent),

	m_vaddrSize (config.getValue<size_t>(*this, "VirtualAddressSize")),
	m_paddrSize (config.getValue<size_t>(*this, "PhysicalAddressSize")),
	m_pidSize (config.getValue<size_t>(*this, "ProcessIdSize")),
	m_enabled (false),
	m_tableAddr (0),
	m_managerAddr (0),
	m_dtlb("dtlb", *this, config),
	m_itlb("itlb", *this, config)
{}

void MMU::Cmd_Info(std::ostream& out, const std::vector<std::string>& /*arguments*/) const{
    out << "The MMU blablabla\n\n";
    out << "                MMU is : " 	<< (m_enabled ? "enabled." : "DISABLED!") << "\n";
    out << "   Virtual address size: " 	<< m_vaddrSize 	<< " bytes\n";
    out << "  Physical address size: " 	<< m_paddrSize 	<< " bytes\n";
    out << "        Process ID size: " 	<< m_pidSize 	<< " bytes\n";
    out << "     Page table address: "  << std::hex << m_tableAddr << "\n";
    out << "        Manager address: "  << std::hex << m_managerAddr << "\n";
    out << "\n";
    out << "Supported operations:\n";
    out << "- enable\n";
    out << "  Enables the MMU\n";
    out << "- disable\n";
    out << "  Disables the MMU\n";
    out << std::flush;

    //MLDTODO Display statistics
}

//MLDQUESTION DoCommand en Read zijn niet de juiste manier. Is dit mogelijk? Hoe? (Trace is lelijk!!!)
void MMU::DoCommand(std::ostream& out, const std::vector<std::string>& arguments){
	if (arguments[0] == "enable"){
		if(m_enabled){
			out << "MMU was already enabled!" << std::endl;
		}else{
			m_enabled = true;
			out << "MMU is now enabled." << std::endl;
		}
	} else if (arguments[0] == "disable"){
		if(m_enabled){
			m_enabled = false;
			out << "MMU is now disabled." << std::endl;
		}else{
			out << "MMU was already disabled!" << std::endl;
		}
	} else {
		out << "Unknown operation: \"" << arguments[0] << "\"" << std::endl;
	}


	//MLDTODO Implement flush operation
	//MLDTODO Implement PID-flush operation
	//MLDTODO Implement invalidate operation
	//MLDTODO Implement set manager address operation
	//MLDTODO implement set pagetable address operation
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
