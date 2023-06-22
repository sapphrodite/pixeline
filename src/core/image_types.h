#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <cassert>
#include <cstddef>
#include <common/integral_types.h>

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

rgba to_rgb(hsv in);
hsv to_hsv(rgba in);

#endif //IMAGE_TYPES_H
