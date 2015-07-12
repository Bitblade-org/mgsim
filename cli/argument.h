#ifndef SIM_ARGUMENT_H_
#define SIM_ARGUMENT_H_

#include <string>
#include <limits>
#include <cstdarg>
#include <algorithm>

#include <arch/simtypes.h>
#include <arch/IOBus.h>
#include <sim/serialization.h>


namespace Simulator{

class Arguments{

public:
	Arguments(const std::vector<std::string> &arguments);

	void expect(size_t nrArguments);
	void expect(size_t nrArgumentsMin, size_t nrArgumentsMax);

	bool namedSet(std::string varName, bool consume, bool &var);
	bool namedSet(std::string varName, bool consume, Addr &addr);
	bool namedSet(std::string varName, bool consume, RAddr &addr);
	bool namedSet(std::string varName, bool consume, IODeviceID &id);

	void set(unsigned int index, bool &var);
	void set(unsigned int index, Addr &addr);
	void set(unsigned int index, RAddr &addr);
	void set(unsigned int index, IODeviceID &id);

	bool getBool(unsigned int index);
	unsigned long long getULL(unsigned int index, unsigned long long max);
	Addr getMAddr(unsigned int index);
	Addr getMAddr(unsigned int index, AddrWidth width);
	RAddr getRMAddr(unsigned int index, AddrWidth width);
	std::string getString(unsigned int index, bool lcase);
	const std::string &getString(unsigned int index); //MLDTODO Ambiguous? Generic transform?

	bool is(unsigned int index, bool caseSensitive, const std::string &cmp);
	bool is(unsigned int index, bool caseSensitive, const std::string &cmp1, const std::string &cmp2);

//	template<typename CMP1, typename... CMPREST>
//	bool is(unsigned int index, bool caseSensitive, const CMP1& cmp1, const CMPREST&... cmpRest);
//	bool is(unsigned int /* index */, bool /* caseSensitive */){ return false; }

	size_t size(){return m_source.size();}
private:
	unsigned long long getULL(const std::string &source, unsigned long long max);
	bool getBool(const std::string &source);

	const std::vector<std::string> &m_source;
	std::map<std::string, std::string> m_namedValues;

};

} /* namespace Simulator */

#endif /* SIM_ARGUMENT_H_ */
