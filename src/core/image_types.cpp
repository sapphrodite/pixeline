#include "image_types.h"
#include <assert.h>

image_t::image_t(vec2D<u16> size) {
	_size = size;
	buf = std::vector<f32>(size.x * size.y * 4, 1.0);
}

vec2D<u16> image_t::size() { return _size; }
rgba image_t::get(vec2D<u16> pos) {
	size_t addr = (pos.x + (pos.y * _size.y)) * 4;
	assert(addr < buf.size());
	return rgba{buf[addr], buf[addr + 1], buf[addr + 2], buf[addr + 3]};
}
void image_t::set(vec2D<u16> pos, rgba r) {
	size_t addr = (pos.x + (pos.y * _size.y)) * 4;
	assert(addr < buf.size());
	for (int i = 0; i < 4; i++)
		buf[addr + i] = r[i];
}
f32* image_t::ptr() { return buf.data(); }
