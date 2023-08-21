#include "image_types.h"
#include <assert.h>
#include <math.h>

bool rgba::operator==(const rgba& rhs) {
	float err = pow(10, -6);
	for (int i = 0; i < 4; i++)
		if (abs(data[i] - rhs.data[i]) > err)
			return false;
	return true;
}

image_t::image_t(vec2u size) {
	_size = size;
	buf = std::vector<f32>(size.x * size.y * 4, 1.0);
}

vec2u image_t::size() { return _size; }
rgba image_t::get(vec2u pos) {
	size_t addr = (pos.x + (pos.y * _size.y)) * 4;
	assert(addr < buf.size());
	return rgba{buf[addr], buf[addr + 1], buf[addr + 2], buf[addr + 3]};
}
void image_t::set(vec2u pos, rgba r) {
	size_t addr = (pos.x + (pos.y * _size.y)) * 4;
	assert(addr < buf.size());
	for (int i = 0; i < 4; i++)
		buf[addr + i] = r[i];
}
f32* image_t::ptr() { return buf.data(); }
