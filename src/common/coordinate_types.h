#ifndef COORDINATE_TYPES_H
#define COORDINATE_TYPES_H

#include <common/integral_types.h>

template <typename T>
struct vec2D {
    vec2D() = default;
    vec2D(T x_in, T y_in ) : x(x_in), y(y_in) {}
    T x, y;

    vec2D& operator+=(const vec2D& rhs)  {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    template <typename U>
    vec2D<U> to() { return vec2D<U>{U(x), U(y)}; }

};

template <typename T>
inline vec2D<T> operator+(vec2D<T> lhs, const vec2D<T>& rhs) {
    lhs += rhs;
    return lhs;
}

template <typename T>
struct rect {
    vec2D<T> origin;
    vec2D<T> size;

    vec2D<T> top_left() const { return origin; }
    vec2D<T> top_right() const { return {origin.x + size.x, origin.y}; }
    vec2D<T> bottom_left() const { return {origin.x, origin.y + size.y}; }
    vec2D<T> bottom_right() const { return {origin.x + size.x, origin.y + size.y}; }
    vec2D<T> center() const { return {origin.x + (size.x / 2), origin.y + (size.y / 2)};}
};

#endif //COORDINATE_TYPES_H
