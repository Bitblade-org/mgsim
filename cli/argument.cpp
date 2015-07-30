#include "argument.h"

namespace Simulator{

Arguments::Arguments(const std::vector<std::string> &arguments):
m_source(arguments),
m_namedValues()
{
	for(size_t i=0; i<arguments.size(); i++){
		std::string str = arguments[i];
		size_t pos = str.find('=');
		if(pos == std::string::npos){
			continue;
		}

		std::string varName = str.substr(0, pos);
		std::string value = str.substr(pos + 1);
		m_namedValues[varName] = value;
	}
}

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
	if(m_source.size() != nrArguments){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected %zu.", m_source.size(), nrArguments);
	}
}

void Arguments::expect(size_t nrArgumentsMin, size_t nrArgumentsMax){
	if(m_source.size() < nrArgumentsMin){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected at least %zu.", m_source.size(), nrArgumentsMin);
	}else if(m_source.size() > nrArgumentsMax){
		throw exceptf<InvalidArgumentException>("Invalid number of arguments. Received %zu, expected at most %zu.", m_source.size(), nrArgumentsMax);
	}
}

bool Arguments::namedSet(std::string varName, bool consume, bool &var){
	if(m_namedValues.count(varName) == 0){
		return false;
	}

	std::string a = m_namedValues[varName];

	if(a == "0" || a == "false" || a == "disabled"){
		var = false;
	}else if(a == "1" || a == "true" || a == "enabled"){
		var = true;
	}else{
		throw exceptf<InvalidArgumentException>("Invalid value `%s` for named argument `%s`: Boolean value was expected.", a.c_str(), varName.c_str());
	}

	if(consume){
		m_namedValues.erase(varName);
	}

	return true;
}

bool Arguments::namedSet(std::string varName, bool consume, Addr &addr){
	if(m_namedValues.count(varName) == 0){
		return false;
	}

	addr = getULL(m_namedValues[varName], std::numeric_limits<Addr>::max());

	if(consume){
		m_namedValues.erase(varName);
	}

	return true;
}

bool Arguments::namedSet(std::string varName, bool consume, RAddr &addr){
	if(m_namedValues.count(varName) == 0){
		return false;
	}

	addr = getULL(m_namedValues[varName], std::numeric_limits<Addr>::max());
	addr.alwaysExpect(addr.m_width);

	if(consume){
		m_namedValues.erase(varName);
	}

	return true;
}

void Arguments::set(unsigned int index, bool &var){
	std::string a = getString(index, true);
	var = getBool(a);
}

void Arguments::set(unsigned int index, Addr &addr){
	addr = getULL(index, std::numeric_limits<Addr>::max());
}

void Arguments::set(unsigned int index, RAddr &addr){
	addr = getULL(index, std::numeric_limits<Addr>::max());
	addr.alwaysExpect(addr.m_width);
}

bool Arguments::getBool(unsigned int index){
	return getBool(m_source[index]);
}

unsigned long long Arguments::getUnsigned(unsigned int index){
	return getULL(m_source[index], ULLONG_MAX);
}

unsigned long long Arguments::getULL(unsigned int index, unsigned long long max){
	return getULL(m_source[index], max);
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
	RAddr addr = RAddr(getMAddr(index), width);
	addr.alwaysExpect(width);
	return addr;
}

std::string Arguments::getString(unsigned int index, bool lcase){
	if(lcase){
		std::string value = m_source[index];
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		return value;
	}

	return m_source[index];
}

const std::string& Arguments::getString(unsigned int index){
	return m_source[index];
}

unsigned long long Arguments::getULL(const std::string &source, unsigned long long max){
	unsigned long long value;
	try{
		value = std::strtoull(source.c_str(), NULL, 0);
	}catch(std::invalid_argument &e){
		throw exceptf<InvalidArgumentException>("Invalid value `%s`: Integer value was expected.", source.c_str());
	}catch(std::out_of_range &e){
		throw exceptf<InvalidArgumentException>("Invalid value `%s`: Integer out of bounds.", source.c_str());
	}

	if(value > max){
		throw exceptf<InvalidArgumentException>("Invalid value `%s`: Value cannot be higher than %llu.", source.c_str(), max);
	}

	return value;
}

bool Arguments::getBool(const std::string &source){
	if(source == "0" || source == "false" || source == "disabled"){
		return false;
	}

	if(source == "1" || source == "true" || source == "enabled"){
		return true;
	}

	throw exceptf<InvalidArgumentException>("Invalid value `%s`: Boolean value was expected.", source.c_str());
}

} /* namespace Simulator */
