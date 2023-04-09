#include "tool.h"
#include <math.h>
void add_pixel(vec2D<i32> px, rect<i32> bound, diff& d) {
    if (px.x >= bound.top_left().x && px.y >= bound.top_left().y 
        && px.y < bound.bottom_right().y && px.x < bound.bottom_right().x)
   d.insert(vec2D<u16>(px.x, px.y), rgba{0, 0, 0, 0});
}

// Brensham's algorithm
diff draw_line(vec2D<i32> a, vec2D<i32> b, rect<i32> bound) {
    diff d;
    vec2D<i32> delta(abs(b.x - a.x), -abs(b.y - a.y));
    vec2D<i32> sign(a.x < b.x ? 1 : -1, a.y < b.y ? 1 : -1);
    i32 error = delta.x + delta.y;

    for (;;) {
        add_pixel(a, bound, d);

        if (a.x == b.x && a.y == b.y) break;
        i32 e2 = 2 * error;
        if (e2 >= delta.y) {
            if (a.x == b.x) break;
            error += delta.y;
            a.x += sign.x;
        } 
        if (e2 <= delta.x) {
            if (a.y == b.y) break;
            error += delta.x;
            a.y += sign.y;
        }
    }
    return d;
}

diff pencil::on_click(vec2D<i32> px, rect<i32> bounds) {
    diff d;
    add_pixel(px, bounds, d);
    _last = px;
    return d;
}

diff pencil::on_drag(vec2D<i32> px, rect<i32> bounds) {
    diff d = draw_line(_last, px, bounds);
    _last = px;
    return d;
}

