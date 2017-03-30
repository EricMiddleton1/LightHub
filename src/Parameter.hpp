#pragma once

#include <string>
#include <functional>
#include <exception>
#include <mutex>

#include "Color.hpp"

class Value
{
public:
	enum class Type {
		Number = 0,
		Bool,
		Color,
		String
	};

	Value(double num);
	Value(bool bl);
	Value(const Color& c);
	Value(const std::string& str);
	virtual ~Value();

	Type getType() const;
	
	double getNumber() const;
	bool getBool() const;
	Color getColor() const;
	std::string getString() const;

	void set(double num);
	void set(const std::string& str);

private:
	Type type;
	double num;
	bool bl;
	Color c;
	std::string str;
};

class Parameter
{
public:
	using ValidationFunction = std::function<std::string(const Value&)>;
	
	class InvalidValue : public std::exception {
	public:
		InvalidValue(const std::string& str);

		virtual const char* what() const noexcept override;
	private:
		std::string msg;
	};

	Parameter(const std::string& name, const Value& val, ValidationFunction validator = {});
	Parameter(const Parameter&);
	
	std::string getName() const;
	Value::Type getType() const;
	Value getDefaultValue() const;
	Value getValue() const;

	void setValue(const Value& val);

	static ValidationFunction ValidatorRange(double min, double max);
	static ValidationFunction ValidatorGreater(double min);
	static ValidationFunction ValidatorLess(double max);

private:
	std::string name;
	Value val, defVal;

	mutable std::mutex mutex;

	ValidationFunction validator;
};
