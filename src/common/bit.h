#ifndef BIT_H
#define BIT_H

#include <array>
#include <vector>
#include <bit>
#include <cstddef>     // size_t
#include <type_traits> // std::conditional
#include <limits>      // std::numeric_limits
#include <cstdint>


class mask {
public:
    using mask_t = uint64_t;
    constexpr mask(mask_t val) : data(val) {}

    constexpr static mask bit(uint8_t idx) { return mask(1 << idx); }
    constexpr static mask range(uint8_t a, uint8_t b) {
        return mask((bit(b - a).value() - 1) << a); }
    constexpr static mask packed(uint8_t idx, uint8_t len) {
        return mask(range(0, len).value() << (idx * len));
    }

    constexpr uint8_t tz() const { return std::countr_zero(data); }
    constexpr mask_t value() const { return data; }
private:
    mask_t data;
};


// TODO - potentially expand *this* to template over a storage type instead?
template <typename T>
class bitstring {
public:
    bitstring() = default;
    constexpr bitstring(T val) : data(val) {}
    T value() const { return data; }

    bool operator==(const bitstring& rhs) const = default;
    bool operator!=(const bitstring& rhs) const = default;
    constexpr T get(mask m) const { return (data & m.value()) >> m.tz(); }
    constexpr void clear(mask m) { data &= ~m.value(); }
    constexpr void write(mask m, T val) {
        clear(m);
        data |= (val << m.tz());
    }
private:
    T data;
};


#endif //BITARRAY_H
