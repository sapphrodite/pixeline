#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include <common/integral_types.h>
#include <common/coordinate_types.h>


struct hsv {
public:
	f32 h, s, v;

	hsv() = default;
	constexpr hsv(f32 h_in, f32 s_in, f32 v_in) : h(h_in), s(s_in), v(v_in) {}
	static constexpr hsv max() { return hsv(360, 100, 100); }
	enum class axis { h, s, v };

	f32& operator[](axis idx) {
		switch(idx) {
			case axis::h: return h;
			case axis::s: return s;
			case axis::v: return v;
			default: throw std::exception();
		}
	}
};

struct rgba {
	u8 r, g, b, a;
	rgba() = default;
	rgba(u8 r_in, u8 g_in, u8 b_in, u8 a_in) : r(r_in), g(g_in), b(b_in), a(a_in) {}

	u8& operator[](size_t index) {
		switch(index) {
			case 0: return r;
			case 1: return g;
			case 2: return b;
			case 3: return a;
			default: throw std::exception();
		}
	}
};

template <typename T>
struct image_wrapper {
public:
	image_wrapper() = default;
	image_wrapper(T data_in, vec2D<u16> size_in) : _data(data_in), _size(size_in) {}
	rgba get(size_t i) {
	 //   assertion(i * 4 + 3 < _data.size(), "Cannot write out of bounds");
		return rgba{_data[i * 4], _data[i * 4 + 1], _data[i * 4 + 2], _data[i * 4 + 3]};
	}
	void write(size_t i, rgba c) {
		// assertion(i * 4 + 3 < _data.size(), "Cannot write out of bounds");
		_data[i * 4] = c.r;
		_data[i * 4 + 1] = c.g;
		_data[i * 4 + 2] = c.b;
		_data[i * 4 + 3] = c.a;
	}
	T& data() { return _data; }
	const T& data() const { return _data; }
	vec2D<u16> size() const { return _size; }
	enum class format {
		rgba,
		rgb
	} fmt = format::rgba;
private:
	T _data;
	vec2D<u16> _size;
};

using image = image_wrapper<std::vector<u8>>;
using framebuffer = image_wrapper<u8*>;

static rgba to_rgb(hsv hsv_in) {
	hsv_in.s /= 100.0f;
	hsv_in.v /= 100.0f;

	const f32 z = hsv_in.v * hsv_in.s;
	const f32 x = z * (1 - fabs(fmod(hsv_in.h / 60.0, 2) - 1));
	const f32 m = hsv_in.v - z;

	f32 rp = 0, gp = 0, bp = 0;

	gp = hsv_in.h >= 60 ? z : x;
	bp = hsv_in.h < 120 ? 0 : x;

	if ((hsv_in.h >= 0) && (hsv_in.h < 60)) {
		rp = z;
		gp = x;
		bp = 0;
	} else if (hsv_in.h >= 60 && hsv_in.h < 120) {
		rp = x;
		gp = z;
		bp = 0;
	} else if (hsv_in.h >= 120 && hsv_in.h < 180) {
		rp = 0;
		gp = z;
		bp = x;
	} else if (hsv_in.h >= 180 && hsv_in.h < 240) {
		rp = 0;
		gp = x;
		bp = z;
	} else if (hsv_in.h >= 240 && hsv_in.h < 300) {
		rp = x;
		gp = 0;
		bp = z;
	} else if (hsv_in.h >= 300 && hsv_in.h <= 360) {
		rp = z;
		gp = 0;
		bp = x;
	}

	return rgba((rp + m) * 255, (gp + m) * 255, (bp + m) * 255, 255);
}

static hsv to_hsv(rgba rgb_in) {
	f32 rp = rgb_in.r / 255.0f;
	f32 gp = rgb_in.g / 255.0f;
	f32 bp = rgb_in.b / 255.0f;

	f32 cmax = std::max({rp, gp, bp});
	f32 cmin = std::min({rp, gp, bp});
	f32 delta = cmax - cmin;

	hsv ret(0, 0, 0);
	if (cmax == rp) {
		ret.h = 60 * fmod((gp - bp) / delta, 6) + 360;
	} else if (cmax == gp) {
		ret.h = 60 * ((bp - rp) / delta) + 120;
	} else if (cmax == bp) {
		ret.h = 60 * ((rp - gp) / delta) + 240;
	}

	ret.s = (1.0f / (cmax / delta));
	ret.v = cmax;
	return hsv(fmod(ret.h, 360), ret.s * 100, ret.v * 100);
}

#endif //IMAGE_TYPES_H
