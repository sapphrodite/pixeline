#include "api.h"
#include "selection.h"
#include <common/coordinate_types.h>

#include <vector>
#include <array>


class layer {
public:
	void apply_diff(diff& d) {
		for (auto [pos, color] : d) {
			size_t base_addr = (pos.x + pos.y * bounds.size.x);

			// if undo diff doesn't have data for the current pixel, back up before overwriting
			if (!undo_diff.exists(pos)) {
				undo_diff.insert(pos, data[base_addr]);
			}

			data[base_addr] = color;
		} 
	}

	void new_image(uint16_t w, uint16_t h) {
		data = std::vector<rgba>(w * h, rgba{1, 1, 1, 1}); 
		bounds = rect<int32_t>{{0, 0}, {w, h}};
	}

	const rgba* ptr() { return data.data(); }
	diff& get_diff() { return undo_diff; }
	void commit() { undo_diff = diff(); }
	rect<int32_t> get_bounds() { return bounds; }
private:
	diff undo_diff;
	std::vector<rgba> data;
	rect<int32_t> bounds;
};

class undo_stack {
public:
	void push(const diff& d) { 
		diffs.resize(++pos);
		diffs[pos - 1] = d;
	}

	void undo(layer& l) {
		if (pos != 0)
			swap_diffs(l, --pos);
	}

	void redo(layer& l) {
		if (pos != diffs.size())
			swap_diffs(l, pos++);
	}
private:
	void swap_diffs(layer& l, size_t idx) {
		l.apply_diff(diffs[idx]);
		diffs[idx] = l.get_diff();
		l.commit();
	}
	std::vector<diff> diffs;
	size_t pos = 0;
};

struct handle {
	std::array<rgba, 256> palette;
	layer canvas;
	undo_stack history;
};



void add_pixel(rgba r, vec2D<int32_t> px, rect<int32_t> bound, diff& d) {
    if (px.x >= bound.top_left().x && px.y >= bound.top_left().y 
        && px.y < bound.bottom_right().y && px.x < bound.bottom_right().x)
   d.insert(vec2D<u16>(px.x, px.y), r);
}

// Brensham's algorithm to draw line from a to b
diff draw_line(rgba r, vec2D<int32_t> a, vec2D<int32_t> b, rect<int32_t> bound) {
    diff d;
    vec2D<int32_t> delta(abs(b.x - a.x), -abs(b.y - a.y));
    vec2D<int32_t> sign(a.x < b.x ? 1 : -1, a.y < b.y ? 1 : -1);
    int32_t error = delta.x + delta.y;

    for (;;) {
        add_pixel(r, a, bound, d);

        if (a.x == b.x && a.y == b.y) break;
        int32_t e2 = 2 * error;
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



handle* handle_alloc() { return new handle; }
void handle_release(handle* hnd) { delete hnd; };

void tool_release(handle* hnd) {
	hnd->history.push(hnd->canvas.get_diff());
	hnd->canvas.commit();
};

void tool_cancel(handle* hnd) {
	hnd->canvas.apply_diff(hnd->canvas.get_diff());
	hnd->canvas.commit();
}

void pencil(handle* hnd, palette_idx c, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	diff d = draw_line(hnd->palette[c], {x1, y1}, {x2, y2}, hnd->canvas.get_bounds());
	hnd->canvas.apply_diff(d);
}

void set_pal_color(handle* hnd, palette_idx c, rgba r) {
	assertion(c < PALETTE_SIZE, "Palette index out of range\n");
	hnd->palette[c] = r; 
}
rgba get_pal_color(handle* hnd, palette_idx c) { 
	assertion(c < PALETTE_SIZE, "Palette index out of range\n");
	return hnd->palette[c];
}

void new_image(handle* hnd, uint16_t w, uint16_t h) { hnd->canvas.new_image(w, h); } 
void get_imagesize(handle* hnd, uint16_t* w, uint16_t* h) { 
	*w = hnd->canvas.get_bounds().size.x;
	*h = hnd->canvas.get_bounds().size.y;
}
const rgba* imagedata(handle* hnd) { return hnd->canvas.ptr(); }


void undo(handle* hnd) { hnd->history.undo(hnd->canvas); }
void redo(handle* hnd) { hnd->history.redo(hnd->canvas); }
