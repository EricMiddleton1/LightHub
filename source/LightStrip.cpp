#include "LightStrip.h"

LightStrip::LightStrip(int count) {
	if (count < 1)
		throw(Exception("LightStrip count must be greater than 0"));

	for (int i = 0; i < count; i++)
		colors.push_back(Color());

	this->size = count;
}

LightStrip::~LightStrip() {
}

void LightStrip::Set(int id, Color c) {
	if (id < 0 || id >= size)
		throw(Exception("LightStrip Set() id must be between 0 and " + size));

	colors[id] = c;
}

void LightStrip::SetAll(Color c) {
	for (int i = 0; i < size; i++)
		colors[i] = c;
}

void LightStrip::ShiftUp(int amount) {

}

void LightStrip::ShiftUp(int start, int amount) {
	auto itr = colors.begin();
	std::copy_backward(itr + start, itr + size - amount, itr + size);
}

void LightStrip::ShiftDown(int amount) {

}

void LightStrip::ShiftDown(int start, int amount) {
	std::vector<Color> tempColor(start - amount);

	auto itr = colors.begin();
	auto tItr = tempColor.begin();

	std::copy(itr + amount, itr + start, tItr);
	std::copy(tItr, tItr + tempColor.size(), itr);
}

Color LightStrip::GetColor(int id) {
	if (id < 0 || id >= size)
		throw(Exception("LightStrip GetColor() id must be between 0 and " + size));

	return colors[id];
}

int LightStrip::GetSize() {
	return size;
}

void LightStrip::DumpColors(unsigned char colorArray[]) {
	for (int i = 0; i < size; i++) {
		Color color = colors[i];

		colorArray[3 * i] = color.GetRed();
		colorArray[3 * i + 1] = color.GetGreen();
		colorArray[3 * i + 2] = color.GetBlue();
	}
}