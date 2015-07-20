#ifndef ARCH_ADDRESS_H_
#define ARCH_ADDRESS_H_

#include <sys_config.h>

#include <stddef.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <climits>

#define RMADDR_STRICT true

#include <sim/log2.h>
#include <sim/serialization.h>

#if defined(TARGET_MTALPHA)
#define ADDR_WIDTH_MAX 64U		//Architecture-dictated maximum width of MAddr
#define PADDR_WIDTH 16			//Width of PAddr
typedef uint64_t Addr; 			//Type capable of storing any valid Addr value
typedef uint8_t  AddrWidth; 	//Type capable of storing any valid Addr width

#elif defined(TARGET_MTSPARC) || defined(TARGET_MIPS32) || defined(TARGET_MIPS32EL) || defined(TARGET_OR1K)
#define ADDR_WIDTH_MAX 32		//Architecture-dictated maximum width of MAddr
#define PADDR_WIDTH 16			//Width of PAddr
typedef uint32_t Addr; 		//Type capable of storing any valid MAddr value (Must be unsigned!)
typedef uint8_t  AddrWidth; 		//Type capable of storing any valid MAddr width
#endif

struct RAddrIndices{
	size_t i;
	size_t j;
};

//Restricted Memory Address
struct RAddr{
	Addr 		m_value;
	AddrWidth 	m_width;

	RAddr():RAddr(0,0){}
	RAddr(int) = delete;
	RAddr(Addr v, AddrWidth w);
	RAddr(void* pointer) : RAddr((Addr)pointer, sizeof(pointer) * CHAR_BIT){}

	static bool  isValid     (Addr const a, AddrWidth const w);
	static void  strictExpect(Addr const a, AddrWidth const w);
	static void  alwaysExpect(Addr const a, AddrWidth const w);
	static RAddr truncateLsb (Addr const a, AddrWidth const oldW, AddrWidth const by); //MLDTODO Deprecate
	static RAddr truncateMsb (Addr const a, AddrWidth const newW);
	static AddrWidth getPrintWidth(AddrWidth addr);

	RAddr truncateLsb(AddrWidth const by)   const {return truncateLsb(m_value, m_width, by);} //MLDTODO Deprecate
	RAddr truncateMsb(AddrWidth const newW) const {return truncateMsb(m_value, newW);}

	bool isValid() 		const {return isValid(m_value, m_width	);}

	void strictExpect(AddrWidth const w) const; //MLDTODO Change to macro
	void alwaysExpect(AddrWidth const w) const;

	AddrWidth getRealWidth(){return ilog2(m_value);}
	AddrWidth getPrintWidth(){return getPrintWidth(m_width);}

	template <typename T> T* ptr(){return (T*)m_value;}

	bool   operator==(RAddr &o) const {return m_value == o.m_value;}
	bool   operator!=(RAddr &o) const {return m_value != o.m_value;}
	RAddr& operator=(const Addr &addr);
	RAddr  operator<<(size_t by) const {return RAddr(m_value << by, m_width + by);}
	RAddr  operator>>(size_t by) const {return RAddr(m_value >> by, m_width - by);}
	RAddr& operator>>=(size_t by) {m_value >>= by; m_width -= by; return *this;}
	RAddr& operator<<=(size_t by) {m_value <<= by; m_width += by; return *this;}
	RAddr& operator|=(const RAddr& o){m_value |= o.m_value; m_width = std::max(m_width, o.m_width); return *this;}
	RAddr& operator&=(const RAddr& o){m_value &= o.m_value; m_width = std::max(m_width, o.m_width); return *this;}
	RAddr  operator&(const RAddr& o) const {return RAddr(m_value & o.m_value, std::max(m_width, o.m_width));}
	RAddr  operator|(const RAddr& o) const {return RAddr(m_value | o.m_value, std::max(m_width, o.m_width));}
	// 00110100 {0,2} = 100
	// 00000001 {0,0} = 1
	// 00110100 {2,5} = 1101
	RAddr operator[](const RAddrIndices& ind){return truncateMsb(m_value >> ind.i, ind.j-ind.i + 1);}
	SERIALIZE(a) {a & "MAddr" & m_value & m_width;}


};

std::ostream& operator<<(std::ostream& os, RAddr ra);

#endif /* ARCH_ADDRESS_H_ */
