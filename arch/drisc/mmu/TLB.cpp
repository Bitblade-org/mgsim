#include "TLB.h"

#include <stddef.h>

#include <sim/config.h>

namespace Simulator {
namespace drisc {
namespace mmu {

TLB::TLB(const std::string& name, Object& parent)
	: Object(name, parent),
	m_numTables (GetConf("NumberOfTables", size_t)),
	m_tables (m_numTables)
{
	m_tables.reserve(m_numTables);

	for( int i=0; i<m_numTables; i++){
		//m_tables.emplace_back("table", *this, config); //MLDTODO Should get this working...
		m_tables.push_back(new Table("table" + std::to_string(i), *this));
	}
}

void TLB::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    out << "The TLB blablabla\n\n";
    out << "  Number of tables: " << unsigned(m_numTables) << "\n\n";
    out << "Supported operations:\n";
    out << "  None implemented yet!" << std::endl;
    //MLDTODO Display statistics
}

void TLB::Cmd_Read(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
	out << "No operations implemented yet!" << std::endl;
	//MLDTODO Implement flush operation
	//MLDTODO Implement PID-flush operation
	//MLDTODO Implement invalidate operation
	//MLDTODO Implement lookup operation (local, auto, force_remote)
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
