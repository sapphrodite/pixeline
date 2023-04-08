#ifndef TOOL_H
#define TOOL_H

#include <common/coordinate_types.h>
#include <vector>

struct diff {
    std::vector<vec2D<u16>> draw_buffer;
};

class tool {
public:
    virtual diff on_click(vec2D<i32> px, rect<i32> bounds) = 0;
    virtual diff on_drag(vec2D<i32> px, rect<i32> bounds) = 0;
};

class pencil {
public:
    diff on_click(vec2D<i32> px, rect<i32> bounds);
    diff on_drag(vec2D<i32> px, rect<i32> bounds);
private:
    vec2D<i32> _last;
};

#endif //TOOL_H
