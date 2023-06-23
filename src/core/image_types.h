#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <cassert>
#include <cstddef>
#include <common/integral_types.h>
#include <core/coordinate_types.h>

struct hsv {
public:
	hsv() = default;
	constexpr hsv(f32 h, f32 s, f32 v) : data(h, s, v) {}
	static constexpr hsv max() { return hsv(360, 100, 100); }

	f32& operator[](size_t idx);

	f32& h() { return data[0]; }
	f32& s() { return data[1]; }
	f32& v() { return data[2]; }
private:
	f32 data[3];
};

struct rgba {
public:
	constexpr rgba() : data(1, 1, 1, 0) {};
	constexpr rgba(f32 r, f32 g, f32 b, f32 a) : data(r, g, b, a) {}

	f32& operator[](size_t idx);

	f32& r() { return data[0]; }
	f32& g() { return data[1]; }
	f32& b() { return data[2]; }
	f32& a() { return data[3]; }
private:
	f32 data[4];
};

class image_format {
public:
	image_format(bool is_float, int num_channels);

	bool is_float() { return data >> 2; }
	int num_channels() { return (data & 0x3) + 1; }
	size_t size_of(int x, int y);
private:
	u64 data;
};

class image_t {
	public:
	image_t(void* data, void* palette, image_format fmt, vec2D<u16> size)
		: data(data), palette(palette), fmt(fmt), size(size) {}
	void* data;
	void* palette;
	image_format fmt;
	vec2D<u16> size;
};

// should prevent copying or moving, copy over the support class
// I bet you there's a bug that occurs on subsequent calls to read_image :)
class png_reader {
public:
	png_reader(const char* filename);
	~png_reader();

	image_format get_fmt();
	size_t width();
	size_t height();
	size_t lenbytes();
	size_t read_image(uint8_t* buf);
private:
	size_t rowbytes();
	struct pimpl;
	pimpl* data = nullptr;
};

rgba to_rgb(hsv in);
hsv to_hsv(rgba in);
void convert_image_fmt(image_t dst, image_t src);

#endif //IMAGE_TYPES_H
