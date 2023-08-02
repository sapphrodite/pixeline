#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <cmath>
#include <cassert>
#include <cstddef>
#include <memory>
#include <common/integral_types.h>
#include <common/coordinate_types.h>

class hsv;
class rgba;


struct hsv {
public:
	constexpr hsv() = default;
	constexpr hsv(f32 h, f32 s, f32 v) : data(h, s, v) {}
	static hsv from(const rgba&);
	static constexpr hsv max() { return hsv(360, 100, 100); }

	f32& operator[](size_t idx) { return data[idx]; };

	f32 h() const { return data[0]; }
	f32 s() const { return data[1]; }
	f32 v() const { return data[2]; }
private:
	f32 data[3];
};

struct rgba {
public:
	constexpr rgba() : data(1, 1, 1, 0) {};
	constexpr rgba(f32 r, f32 g, f32 b, f32 a) : data(r, g, b, a) {}
	static rgba from(const hsv&);

	f32& operator[](size_t idx) { return data[idx]; };
	bool operator==(const rgba& rhs) {
		float err = pow(10, -6);
		for (int i = 0; i < 4; i++)
			if (abs(data[i] - rhs.data[i]) > err)
				return false;
		return true;
	}

	f32 r() const { return data[0]; }
	f32 g() const { return data[1]; }
	f32 b() const { return data[2]; }
	f32 a() const { return data[3]; }
private:
	f32 data[4];
};

class image_format {
public:
	#define BUFTYPES(m) m(u8) m(u16) m(f32)
	enum class buf_t {
		u8, u16, f32
	};

	image_format() = default;
	image_format(buf_t buftype, int num_channels);

	buf_t buftype() { return _buftype; }
	bool is(buf_t T) { return _buftype == T; }

	int num_channels();
	size_t size_of(int x, int y);
private:
	buf_t _buftype;
	u64 data = 0;
};

class image_t {
public:
	image_t(image_format fmt, vec2D<u16> size);
	image_t();
	~image_t();
	image_t& operator=(image_t&& other);
	image_t(image_t&& other);

	static image_t from_file(const char* filename);
	image_t convert_to(image_format dstfmt);

	rgba at(vec2D<u16>);
	rgba at(size_t);
	f32* buf();

	vec2D<u16> size();
private:
	struct pimpl;
	std::unique_ptr<pimpl> data;
};

#endif //IMAGE_TYPES_H
