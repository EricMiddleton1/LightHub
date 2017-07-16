#include "Parameter.hpp"


Value::Value(int _Int)
	:	type{Type::Int}
	,	Int{_Int} {
}

Value::Value(double _Double)
	:	type{Type::Double}
	,	Double{_Double} {
}

Value::Value(bool _bl)
	:	type{Type::Bool}
	,	bl{_bl} {
}

Value::Value(const Color& _c)
	:	type{Type::Color}
	,	c{_c} {
}

Value::Value(const std::string& _str)
	:	type{Type::String}
	,	str{_str} {
}

Value::~Value() {
}

Value::Type Value::getType() const {
	return type;
}

int Value::getInt() const {
	if(type != Type::Int) {
		throw std::runtime_error("Value::getInt: Value is not of type Int");
	}

	return Int;
}

double Value::getDouble() const {
	if(type != Type::Double) {
		throw std::runtime_error("Value::getDouble: Value is not of type Double");
	}

	return Double;
}

bool Value::getBool() const {
	if(type != Type::Bool) {
		throw std::runtime_error("Value::getBool: Value is not of type Bool");
	}

	return bl;
}

Color Value::getColor() const {
	if(type != Type::Color) {
		throw std::runtime_error("Value::getColor: Value is not of type Color");
	}

	return c;
}

std::string Value::getString() const {
	if(type != Type::String) {
		throw std::runtime_error("Value::getString: Value is not of type String");
	}

	return str;
}

void Value::set(int _Int) {
	if(type != Type::Int) {
		throw std::runtime_error("Value::set(int): Value is not of type Int");
	}

	Int = _Int;
}


void Value::set(double _Double) {
	if(type != Type::Double) {
		throw std::runtime_error("Value::set(double): Value is not of type Double");
	}

	Double = _Double;
}

void Value::set(const std::string& _str) {
	if(type != Type::String) {
		throw std::runtime_error("Value::set(string): Value is not of type String");
	}

	str = _str;
}

Parameter::InvalidValue::InvalidValue(const std::string& str)
	:	msg{str} {
}

const char* Parameter::InvalidValue::what() const noexcept {
	return msg.c_str();
}

Parameter::Parameter(const std::string& _name, const Value& _val,
	Parameter::ValidationFunction _validator)
	:	name{_name}
	,	val{_val}
	,	defVal{_val} 
	,	validator{_validator} {
}

Parameter::Parameter(const Parameter& p)
	:	name{p.name}
	,	val{p.val}
	,	defVal{p.defVal}
	,	validator{p.validator} {
}

std::string Parameter::getName() const {
	std::unique_lock<std::mutex> lock(mutex);

	return name;
}

Value::Type Parameter::getType() const {
	std::unique_lock<std::mutex> lock(mutex);

	return val.getType();
}

Value Parameter::getDefaultValue() const {
	std::unique_lock<std::mutex> lock(mutex);

	return defVal;
}

Value Parameter::getValue() const {
	std::unique_lock<std::mutex> lock(mutex);

	return val;
}

void Parameter::setValue(const Value& _val) {
	std::unique_lock<std::mutex> lock(mutex);

	if(_val.getType() != val.getType()) {
		throw std::invalid_argument("Paramter:setValue: Incompatible value type");
	}
	
	if(validator) {
		std::string error(validator(_val));

		if(error.length() > 0) {
			throw InvalidValue(error);
		}
	}

	val = _val;
}

Parameter::ValidationFunction Parameter::ValidatorRange(double _min, double _max) {
	return [_min, _max](const Value& val) {
		if(val.getType() != Value::Type::Double) {
			return std::string("Value must be a number");
		}
		else if(val.getDouble() < _min || val.getDouble() > _max) {
			return "Value must be in range [" + std::to_string(_min) + ", " +
				std::to_string(_max) + "]";
		}
		else {
			return std::string();
		}
	};
}

Parameter::ValidationFunction Parameter::ValidatorGreater(double _min) {
	return [_min](const Value& val) {
		if(val.getType() != Value::Type::Double) {
			return std::string("Value must be a number");
		}
		else if(val.getDouble() <= _min) {
			return "Value must be greater than " + std::to_string(_min);
		}
		else {
			return std::string();
		}
	};
}

Parameter::ValidationFunction Parameter::ValidatorLess(double _max) {
	return [_max](const Value& val) {
		if(val.getType() != Value::Type::Double) {
			return std::string("Value must be a number");
		}
		else if(val.getDouble() >= _max) {
			return "Value must be less than " + std::to_string(_max);
		}
		else {
			return std::string();
		}
	};
}

