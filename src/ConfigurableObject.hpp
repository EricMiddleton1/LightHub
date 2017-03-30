#pragma once

#include <vector>

#include "Parameter.hpp"

class ConfigurableObject
{
public:
	ConfigurableObject(const std::vector<Parameter>& parameters);

	Parameter getParameter(const std::string& name) const;
	void setParameter(const std::string& name, const Value& val);

protected:
	std::vector<Parameter> parameters;
};
