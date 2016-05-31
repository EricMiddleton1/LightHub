#pragma once

#include "Color.h"
#include <exception>
#include <string>
#include <vector>

class LightStrip
{
public:
	LightStrip() {};
	LightStrip(int count);
	~LightStrip();

	void Set(int id, Color c);
	void SetAll(Color c);

	void ShiftUp(int amount);
	void ShiftUp(int start, int amount);

	void ShiftDown(int amount);
	void ShiftDown(int start, int amount);

	int GetSize();
	Color GetColor(int id);

	void DumpColors(unsigned char colorArray[]);

	class Exception : std::exception {
	public:
		Exception(std::string message) { _msg = std::string(message); }

		virtual const char *what() { return _msg.c_str(); }
	private:
		std::string _msg;
	};
private:
	std::vector<Color> colors;
	int size;

	void MoveColors(int start, int end, int amount);
};