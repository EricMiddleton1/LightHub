#include "ConfigurableObject.hpp"

#include <algorithm>
#include <stdexcept>

ConfigurableObject::ConfigurableObject(const std::vector<Parameter>& _parameters)
	:	parameters{_parameters} {
}

Parameter ConfigurableObject::getParameter(const std::string& name) const {
	auto found = std::find_if(parameters.begin(), parameters.end(),
		[&name](const Parameter& param) {
			return param.getName() == name;
		});

	if(found == parameters.end()) {
		throw std::runtime_error("ConfigurableObject::getParameter: Parameter '" + name +
			"' not found");
	}

	return *found;
}

void ConfigurableObject::setParameter(const std::string& name, const Value& val) {
	auto found = std::find_if(parameters.begin(), parameters.end(),
		[&name](const Parameter& param) {
			return param.getName() == name;
		});

	if(found == parameters.end()) {
		throw std::runtime_error("ConfigurableObject::getParameter: Parameter '" + name +
			"' not found");
	}

	found->setValue(val);
}
