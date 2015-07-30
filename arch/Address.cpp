#include "Address.h"

#include <stddef.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ios>

RAddr::RAddr(Addr val, AddrWidth w): m_value(val), m_width(w)
{
#if (RMADDR_STRICT == true)
	if(!isValid(val, w)){
		std::stringstream ss;
		ss << "Invalid address for RMAddr[" << w << "]: " << std::hex << std::showbase << val;
		throw std::invalid_argument(ss.str());
	}
#endif
}

RAddr RAddr::truncateLsb(Addr const a, AddrWidth const oldW, AddrWidth const by)
{
	return RAddr((a >> by), oldW - by);
}

RAddr RAddr::truncateMsb(Addr const a, AddrWidth const newW)
{
	return RAddr((a & (~((~Addr(0)) << newW))), newW);
}

bool RAddr::isValid(Addr const a, AddrWidth const w)
{
	static constexpr int lim = std::numeric_limits<Addr>::digits;
	if(lim >= w){
		return true;
	}
	return ((a >> w) == 0);
}

void RAddr::alwaysExpect(Addr const a, AddrWidth const w ){
	if(!isValid(a, w)){
		std::stringstream ss;
		ss << "Address is invalid (width mismatch) MAddr[" << w << "]: " << std::hex << std::showbase << a;
		throw std::invalid_argument(ss.str());
	}
}

void RAddr::strictExpect(Addr const a, AddrWidth const w ){
#if (RMADDR_STRICT == true)
	alwaysExpect(a, w);
#endif
}

void RAddr::alwaysExpect(AddrWidth const w) const{
	if(m_width != w){
		std::stringstream ss;
		ss << "Address has width " << m_width << " while " << w << " was expected";
		throw std::invalid_argument(ss.str());
	}

	if(!isValid()){
		std::stringstream ss;
		ss << "Address is invalid (width mismatch) RMAddr[" << m_width << "]: " << std::hex << std::showbase << m_value;
		throw std::invalid_argument(ss.str());
	}
}

void RAddr::strictExpect(AddrWidth const w) const{
#if (RMADDR_STRICT == true)
	alwaysExpect(w);
#endif
}

AddrWidth RAddr::getPrintWidth(AddrWidth w){
	AddrWidth width = (w + 3) / 4; // ceil(w/4), width of hex value
	width += 5; // [..:0x...]
	width += w > 0 ? std::log10(w) + 1 : 1; // Width display
	return width;
}

std::ostream& operator<<(std::ostream& os, RAddr addr)
{
	std::ios init(NULL);
	init.copyfmt(os);

	size_t hexWidth = (addr.m_width + 3) / 4; // ceil(addr.m_width/4)

	if(addr.isValid()){
		os << "[" << std::dec << unsigned(addr.m_width) << ":0x";
		os << std::hex << std::noshowbase << std::uppercase << std::right;
		os << std::setw(hexWidth) << std::setfill('0') << addr.m_value << "]";
	}else{
		AddrWidth real = addr.getRealWidth();
		AddrWidth expected = addr.m_width;
		AddrWidth truncateBy = real - expected;
		addr = addr.truncateLsb(addr.m_value, real, truncateBy);

		os << "!" << std::dec << unsigned(addr.m_width) << ":0x";
		os << std::hex << std::noshowbase << std::uppercase << std::right;
		os << std::setw(hexWidth) << std::setfill('0') << addr.m_value << "..";
	}

    os.copyfmt(init);
	return os;
}

RAddr& RAddr::operator=(const Addr &addr){
	RAddr::strictExpect(addr, m_width);
	m_value = addr;
	return *this;
}
