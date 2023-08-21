#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <include/coordinate_types.h>
#include <include/integral_types.h>
#include <vector>

struct rgba {
public:
	constexpr rgba() : data(1, 1, 1, 0) {};
	constexpr rgba(f32 r, f32 g, f32 b, f32 a) : data(r, g, b, a) {}

	f32 operator[](size_t idx) { return data[idx]; };
	bool operator==(const rgba& rhs);

	f32 r() const { return data[0]; }
	f32 g() const { return data[1]; }
	f32 b() const { return data[2]; }
	f32 a() const { return data[3]; }
private:
	f32 data[4];
};

class image_t {
public:
	image_t() = default;
	image_t(vec2u size);

	vec2u size();
	rgba get(vec2u);
	void set(vec2u, rgba);
	f32* ptr();
private:
	std::vector<f32> buf;
	vec2u _size;
};

#endif //IMAGE_TYPES_H
