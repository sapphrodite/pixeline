#include "api.h"
#include "diff.h"
#include <include/coordinate_types.h>
#include <vector>
#include <array>

struct handle {
	std::array<rgba, 256> palette;
	image_t canvas;
	bool tool_active = false;
	u8 active_color = 0;
	vec2D<int> last_pos = vec2D<int>{-1, -1};
};

void apply_diff(image_t& img, diff& d) {
	for (auto [pos, color] : d) {
		img.set(pos, color);
	} 
}

// Brensham's algorithm to draw line from a to b
diff draw_line(rgba r, vec2D<int> a, vec2D<int> b, rect<int> bound) {
	diff d;
	vec2D<int> delta(abs(b.x - a.x), -abs(b.y - a.y));
	vec2D<int> sign(a.x < b.x ? 1 : -1, a.y < b.y ? 1 : -1);
	int error = delta.x + delta.y;

	for (;;) {
		if (bound.contains(a))
			d.insert(a.to<u16>(), r);

		if (a.x == b.x && a.y == b.y) break;
		int e2 = 2 * error;
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

handle* handle_new() { return new handle; }
void handle_free(handle* hnd) { delete hnd; };

void cursor_press(handle* hnd, int x, int y) {
	pencil(hnd, hnd->active_color, x, y, x, y);
	hnd->tool_active = true;
	hnd->last_pos = vec2D<int>{x, y};
}

void cursor_drag(handle* hnd, int x, int y) {
	if (hnd->tool_active) {
		pencil(hnd, hnd->active_color, hnd->last_pos.x, hnd->last_pos.y, x, y);
		hnd->last_pos = vec2D<int>{x, y};
	}
}

void cursor_release(handle* hnd) { hnd->tool_active = false; }

void pencil(handle* hnd, u8 pal_idx, int x1, int y1, int x2, int y2) {
	rect<int> bounds{{0, 0}, hnd->canvas.size().to<int>()};
	diff d = draw_line(hnd->palette[pal_idx], {x1, y1}, {x2, y2}, bounds);
	apply_diff(hnd->canvas, d);
}

void pal_select(handle* hnd, u8 pal_idx) { hnd->active_color = pal_idx; }
void pal_set(handle* hnd, u8 pal_idx, rgba r) { hnd->palette[pal_idx] = r; }
rgba pal_get(handle* hnd, u8 pal_idx) { return hnd->palette[pal_idx]; }

void image_new(handle* hnd, u16 w, u16 h) { hnd->canvas = image_t({w, h}); }
void image_size(handle* hnd, u16* w, u16* h) {
	*w = hnd->canvas.size().x;
	*h = hnd->canvas.size().y;
}
const f32* image_data(handle* hnd) { return hnd->canvas.ptr(); }
