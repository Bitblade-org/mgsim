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
#include <arch/IOBus.h>
#include "sim/kernel.h"
#include "manager/MgtMsg.h"
#include "sim/flag.h"
#include "sim/buffer.h"


#include "Table.h"


namespace Simulator {
namespace drisc {
namespace mmu {

using manager::MgtMsg;
using manager::MgtMsgType;
//MLDNOTE Not extending MMIOComponent. TLB is not a memory component itself. Table probably is.
class TLB :	public Object, public IIOBusClient, public Inspect::Interface<Inspect::Info | Inspect::Write> {

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
    TLB(const std::string& name, Object& parent, IIOBus* iobus);
    ~TLB();

    bool invoke(){ return true; } //MLDTODO Implement invoke
    bool OnReadRequestReceived(IODeviceID from, MemAddr address, MemSize size);
	bool OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data);

    Result lookup(RAddr const processId, RAddr const vAddr, RAddr &d$line, bool &r, bool &w, RAddr &pAddr, bool mayUnlock);
    AddrWidth getMinOffsetSize(){return m_tables[0]->getOffsetWidth();}

    Result onInvalidateMsg(MgtMsg &msg);
    Result onPropertyMsg(MgtMsg &msg);
    Result onStoreMsg(MgtMsg &msg);

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments) override;
    void Cmd_Usage(std::ostream& out) const;

    const std::string& GetIODeviceName() const {return GetName();}
    void GetDeviceIdentity(IODeviceIdentification& id) const {id = IODeviceIdentification{1,10,1};} //MLDTODO Figure out what this does
private:
    Result handleMgtMsg(MgtMsg msg);

    void unlink(Line &line);
    void invalidate();
    void invalidate(RAddr pid);
    void invalidate(RAddr pid, RAddr addr);

    Result store_entry(Line &line);
    Result store_pending_entry(RAddr processId, RAddr vAddr, int D$line);

    Line* doLookup(RAddr processId, RAddr vAddr, LineTag tag);

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

	IODeviceID 		m_ioDevId;
	IIOBus&			m_ioBus;
	IoAddr			m_mgtAddr;

    uint8_t	const		m_numTables;
    std::vector<Table*>	m_tables;
    bool				m_enabled;

    Object& GetDRISCParent() const { return *GetParent()->GetParent(); }
   	MMU&	getMMU() const { return (MMU&)*GetParent(); }
};

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TLB_H_ */
