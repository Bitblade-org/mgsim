#include "argument.h"

namespace Simulator{

bool Arguments::is(unsigned int index, bool caseSensitive, const std::string &cmp){
	std::string value = getString(index, caseSensitive);
	return (value == cmp);
}

bool Arguments::is(unsigned int index, bool caseSensitive, const std::string &cmp1, const std::string &cmp2){
	return (is(index, caseSensitive, cmp1) || is(index, caseSensitive, cmp2));
}

//MLDTODO Would be cool if I got this to work
//template<typename CMP1, typename... CMPREST>
//bool Arguments::is(unsigned int index, bool caseSensitive, const CMP1& cmp1, const CMPREST&... cmpRest){
//	return is(index, caseSensitive, cmp1) || is(index, caseSensitive, cmpRest...);
//}

void Arguments::expect(size_t nrArguments){
	if(m_arguments.size() != nrArguments){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected %zu.", m_arguments.size(), nrArguments);
	}
}

void Arguments::expect(size_t nrArgumentsMin, size_t nrArgumentsMax){
	if(m_arguments.size() < nrArgumentsMin){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected at least %zu.", m_arguments.size(), nrArgumentsMin);
	}else if(m_arguments.size() > nrArgumentsMax){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected at most %zu.", m_arguments.size(), nrArgumentsMax);
	}
}

void Arguments::set(unsigned int index, bool &var){
	std::string a = getString(index, true);

	if(a == "0" || a == "false" || a == "disabled"){
		var = false;
		return;
	}

	if(a == "1" || a == "true" || a == "enabled"){
		var = true;
		return;
	}

	throw exceptf<InvalidArgumentException>("Invalid value `%s` for argument %d: Boolean value was expected.", a.c_str(), index);
}

void Arguments::set(unsigned int index, Addr &addr){
	addr = getULL(index, std::numeric_limits<Addr>::max());

}
void Arguments::set(unsigned int index, RAddr &addr){
	addr = getULL(index, std::numeric_limits<Addr>::max());
	addr.alwaysExpect(addr.m_width);
}

void Arguments::set(unsigned int index, IODeviceID &id){
	id = getULL(index, std::numeric_limits<IODeviceID>::max());
	//MLDTODO Test if id is correct!
}

bool Arguments::getBool(unsigned int index){
	std::string a = m_arguments[index];
	if(a == "0" || a == "false" || a == "disabled"){
		return false;
	}

	if(a == "1" || a == "true" || a == "enabled"){
		return true;
	}

	throw exceptf<InvalidArgumentException>("Invalid value `%s` for argument %d: Boolean value was expected.", a.c_str(), index);
}

unsigned long long Arguments::getULL(unsigned int index, unsigned long long max){
	std::string a = m_arguments[index];

	unsigned long long value;
	try{
		value = std::strtoull(a.c_str(), NULL, 0);
	}catch(std::invalid_argument &e){
		throw exceptf<InvalidArgumentException>("Invalid value `%s` for argument %u: Integer value was expected.", a.c_str(), index);
	}catch(std::out_of_range &e){
		throw exceptf<InvalidArgumentException>("Invalid value `%s` for argument %u: Integer out of bounds.", a.c_str(), index);
	}

	if(value > max){
		throw exceptf<InvalidArgumentException>("Invalid value `%s` for argument %u: Value cannot be higher than %llu.", a.c_str(), index, max);
	}

	return value;
}

Addr Arguments::getMAddr(unsigned int index){
	return getULL(index, std::numeric_limits<Addr>::max());
}

Addr Arguments::getMAddr(unsigned int index, AddrWidth width){
	Addr addr = getULL(index, std::numeric_limits<Addr>::max());
	RAddr::alwaysExpect(addr, width);
	return addr;
}

RAddr Arguments::getRMAddr(unsigned int index, AddrWidth width){
	RAddr addr = RAddr(width, getMAddr(index));
	addr.alwaysExpect(width);
	return addr;
}

std::string Arguments::getString(unsigned int index, bool lcase){
	if(lcase){
		std::string value = m_arguments[index];
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		return value;
	}

	return m_arguments[index];
}

const std::string& Arguments::getString(unsigned int index){
	return m_arguments[index];
}

} /* namespace Simulator */
