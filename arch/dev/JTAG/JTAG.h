#ifndef ARCH_DEV_JTAG_JTAG_H_
#define ARCH_DEV_JTAG_JTAG_H_

#define JTAG_CPU 2

#include <string>
#include <iostream>
#include <iomanip>
#include <sim/kernel.h>
#include <arch/Address.h>
#include "RefillMessage.h"
#include <arch/IOBus.h>
#include "sim/flag.h"
#include "manager/MgtMsg.h"
#include <sim/buffer.h>
#include <sim/serialization.h>




namespace Simulator {
using manager::MgtMsg;
using manager::MgtMsgType;

class TestNet: public Object{

public:
	TestNet(const std::string& name, Object& parent);
	static void d$Push(Addr line);
};

class MMUTester;
class MgrTester;
class JTAG: public Object, public IIOBusClient {

	struct IoAddr{
	     IODeviceID devid;
	     IONotificationChannelID chan;
	     SERIALIZE(__a) {__a & "[1111";__a & devid;__a & chan;__a & "]";}
	};


	struct IoMsg{
		IoAddr 		addr;
		uint64_t 	payload;
	    SERIALIZE(__a) {__a & "[2222";__a & addr;__a & payload;__a & "]";}
	};

public:
	JTAG(const std::string& name, Object& parent, IIOBus& iobus, IODeviceID devid);
	JTAG(const JTAG&) = delete;

	static JTAG& getJTAG(JTAG* ref = NULL);

	bool OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data);

	Result sendMgtMsg(MgtMsg msg);


	void start();
	Object* findObject(Object* base, const std::string& name);

	void processTester();

	static void sectionStart(const std::string &name);
	static void sectionEnd(const std::string &name);
	static void printBanner(const std::string &text);
	static void printHeader(const std::string &text);

	MMUTester& getMMUTester(){ return *m_mmuTester; };
	MgrTester& getMgrTester(){ return *m_mgrTester; };

	JTAG& operator=(const JTAG&) = delete;

	static int s_indent;

    const std::string& GetIODeviceName() const {return GetName();}
    void GetDeviceIdentity(IODeviceIdentification& id) const {id = IODeviceIdentification{0,0,0};} //MLDTODO Figure out what this does

private:
    void startMgrTest();
    Result doTransmit();
    Result doReceive();
    void handleMgtMsg(MgtMsg msg);

    Buffer<IoMsg> 	m_fifo_out;
    Buffer<IoMsg>	m_fifo_in;
    //Flag			m_transmitting;
    Process			p_transmit;

    Process			p_receive;
    Flag			m_receiving;

	MMUTester* 		m_mmuTester;
	MgrTester* 		m_mgrTester;
	TestNet	   		m_testNet;
	IODeviceID 		m_ioDevId;
	IIOBus&			m_ioBus;
	IoAddr			m_mgtAddr;
	MgtMsg			m_mgtMsgBuffer;

    Process p_dummy;
    Result DoNothing() { COMMIT{ p_dummy.Deactivate(); }; return SUCCESS; }
};

} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_JTAG_H_ */
