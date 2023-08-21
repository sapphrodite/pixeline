#ifndef COORDINATE_TYPES_H
#define COORDINATE_TYPES_H

template <typename T = int>
struct vec2 {
	vec2() = default;
	vec2(T x_in, T y_in) : x(x_in), y(y_in) {}
	T x, y;

	template <typename U>
	vec2<U> to() { return vec2<U>{U(x), U(y)}; }
};

template <typename T = int>
struct rect_t {
	vec2<T> origin;
	vec2<T> size;

	vec2<T> top_left() const { return origin; }
	vec2<T> top_right() const { return {origin.x + size.x, origin.y}; }
	vec2<T> bottom_left() const { return {origin.x, origin.y + size.y}; }
	vec2<T> bottom_right() const { return {origin.x + size.x, origin.y + size.y}; }
	vec2<T> center() const { return {origin.x + (size.x / 2), origin.y + (size.y / 2)};}
	bool contains(vec2<T> p) {
		bool x = p.x >= origin.x && p.x < origin.x + size.x;
		bool y = p.y >= origin.y && p.y < origin.y + size.y;
		return x && y;
	}
};

using rect = rect_t<int>;
using vec2i = vec2<int>;
using vec2u = vec2<unsigned>;

#define VEC2_ARITH(op) \
template <typename T> \
vec2<T> operator op (vec2<T> lhs, const vec2<T>& rhs) { \
	lhs op##= rhs; \
	return lhs; \
}

#define VEC2_COMPOUNDARITH(op) \
template <typename T> \
vec2<T> operator op##=(vec2<T>& lhs, const vec2<T>& rhs) { \
	lhs.x op##= rhs.x; \
	lhs.y op##= rhs.y; \
	return lhs; \
}

#define VEC2_DEFINE_OP(op) \
	VEC2_ARITH(op) \
	VEC2_COMPOUNDARITH(op)

VEC2_DEFINE_OP(+)
VEC2_DEFINE_OP(-)
VEC2_DEFINE_OP(*)
VEC2_DEFINE_OP(/)

#endif //COORDINATE_TYPES_H
