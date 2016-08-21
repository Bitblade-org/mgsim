#ifndef ARCH_DRISC_MMU_TLB_H_
#define ARCH_DRISC_MMU_TLB_H_

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cassert>

#include <sim/inspect.h>
#include <cli/argument.h>
#include <arch/simtypes.h>
#include "sim/kernel.h"
#include "manager/MgtMsg.h"
#include "sim/flag.h"
#include "sim/buffer.h"
#include <arch/IOMessageInterface.h>

#include "Table.h"

#define INVALID_TLB_LINE_INDEX UINT_MAX

namespace Simulator {
namespace drisc {
namespace mmu {

using manager::MgtMsg;
using manager::MgtMsgType;

struct TLBResultMessage{
	bool pending;
	bool read;
	bool write;
	MemAddr pAddr;
	MemSize tlbOffsetWidth;
};

struct TlbLineRef{
	Line* 	m_line;
	Table*	m_table;
};

//MLDNOTE Not extending MMIOComponent. TLB is not a memory component itself. Table probably is.
class TLB :	public IIOMessageClient, public Object, public Inspect::Interface<Inspect::Info | Inspect::Write> {

	//MLDTODO Create .p.h file
	struct IoAddr{
	     IODeviceID devid;
	     IONotificationChannelID chan;
	     SERIALIZE(__a) {__a & "[1111";__a & devid;__a & chan;__a & "]";}
	};

	//MLDTODO Create .p.h file
	struct IoMsg{
		IoAddr 		addr;
		uint64_t 	payload;
	    SERIALIZE(__a) {__a & "[2222";__a & addr;__a & payload;__a & "]";}
	};

//MLDTODO Keep statistics!
	/*
	 * - Number of hits
	 * - Number of misses
	 * 		- line ==, etc
	 * - Number of flushes
	 * - Number of PID invalidates
	 * - Number of vAddr+PID invalidates
	 *
	 *
	 *
	 * - Per lookup: Manager round trip time
	 */
public:
    TLB(const std::string& name, Object& parent, IOMessageInterface& ioif);
    TLB(const TLB&) = delete;
    ~TLB();

    bool invoke(){ return true; } //MLDTODO Implement invoke
	bool OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data);

	TLBResultMessage getLine(TlbLineRef lineRef);

	Result lookup(Addr contextId, Addr vAddr, bool mayUnlock, TLBResultMessage &res);
	AddrWidth getMinOffsetWidth(){return m_tables[0]->getOffsetWidth();}
	AddrWidth getMaxOffsetWidth(){return m_tables[m_numTables-1]->getOffsetWidth();}


	void setDCacheReference(unsigned dRef);

    Result onInvalidateMsg(MgtMsg &msg);
    Result onPropertyMsg(MgtMsg &msg);
    Result onStoreMsg(MgtMsg &msg);

    bool isEnabled() {return m_enabled;}

    void GetDeviceIdentity(IODeviceIdentification& id) const;
    const std::string& GetIODeviceName() const;

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments) override;
    void Cmd_Usage(std::ostream& out) const;

    void operator=(TLB&) = delete;
    void invalidate();

private:
	Result lookup(RAddr const contextId, RAddr const vAddr, bool mayUnlock, TLBResultMessage &res);
	Result loopback(Addr contextId, Addr vAddr, TLBResultMessage &res);

    Result handleMgtMsg(MgtMsg msg);

    void unlink(Line &line);
    void invalidate(RAddr pid);
    void invalidate(RAddr pid, RAddr addr);

    Result store_entry(Line &line);
    Result store_pending_entry(RAddr contextId, RAddr vAddr, int D$line);

    TlbLineRef doLookup(RAddr contextId, RAddr vAddr, LineTag tag);

    IOMessageInterface&	m_ioif;
	IODeviceID 		m_ioDevId;
    Clock&			m_clock;
    Process			p_transmit;
    Buffer<IoMsg> 	m_fifo_out;
    Result 			doTransmit();
	bool 			sendMgtMsg(MgtMsg msg);

    Process			p_receive;
    Buffer<IoMsg>	m_fifo_in;
    Flag			m_receiving;
	MgtMsg			m_mgtMsgBuffer;
    Result 			doReceive();
    Process 		p_dummy;
    Result 			DoNothing() { COMMIT{ p_dummy.Deactivate(); }; return SUCCESS; }

	IoAddr			m_mgtAddr;

    uint8_t	const		m_numTables;
    std::vector<Table*>	m_tables;
    bool				m_enabled;
    Line*		m_lastLine;

    DRISC*  GetDRISCParent() { return (DRISC*)GetParent()->GetParent(); }
   	MMU&	getMMU() const { return (MMU&)*GetParent(); }
};

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TLB_H_ */
