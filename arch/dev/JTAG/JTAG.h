#ifndef ARCH_DEV_JTAG_JTAG_H_
#define ARCH_DEV_JTAG_JTAG_H_

#define JTAG_CPU 4

#include <string>
#include <iostream>
#include <iomanip>
#include <sim/kernel.h>
#include <arch/Address.h>
#include <arch/drisc/RemoteMessage.h>
#include "RefillMessage.h"


namespace Simulator {
using drisc::RemoteMessage;

class TestNet: public Object {

public:
	TestNet(const std::string& name, Object& parent);
	static void mgrPush(const RemoteMessage &msg);
	static void netPushRefill(const tlbRefillMsg &msg);
	static tlbRefillMsg netPopRefill();
	static void d$Push(Addr line);

private:
	static tlbRefillMsg s_netMsg;
};


class MMUTester;
class MgrTester;
class JTAG: public Object {

public:
	JTAG(const std::string& name, Object& parent);
	JTAG(const JTAG&) = delete;

	static JTAG& getJTAG(JTAG* ref = NULL);

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

private:
	MMUTester* m_mmuTester;
	MgrTester* m_mgrTester;
	TestNet	   m_testNet;

};

} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_JTAG_H_ */
