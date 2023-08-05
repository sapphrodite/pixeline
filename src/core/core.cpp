#include "api.h"
#include "selection.h"
#include <common/coordinate_types.h>

#include <vector>
#include <array>

class commit {
public:
	std::vector<diff> diffs;
};

class layer {
public:
	void apply_diff(int layer_id, diff& d) {
		if (undo_commit.diffs.size() <= layer_id)
			undo_commit.diffs.resize(layer_id + 1);
		diff& undo_diff = undo_commit.diffs[layer_id];

		for (auto [pos, color] : d) {
			size_t addr = (pos.x + pos.y * bounds.size.x);

			// if undo diff doesn't have data for the current pixel, back up before overwriting
			if (!undo_diff.exists(pos)) {
				undo_diff.insert(pos, layers[active_layer].at(addr));
			}

			f32* ptr = layers[active_layer].buf();
			ptr[addr * 4 + 0] = color.r();
			ptr[addr * 4 + 1] = color.g();
			ptr[addr * 4 + 2] = color.b();
			ptr[addr * 4 + 3] = color.a();

			composite_pixel(pos.to<int>());
		} 
	}

	void apply_commit(commit& c) {
		for (int i = 0; i < layers.size(); i++)
			apply_diff(i, c.diffs[i]);
	}

	void new_image(uint16_t w, uint16_t h) {
		// TODO - this should clear all existing data and history
		layers.emplace_back(image_t(canvas_fmt(), vec2D<u16>(w, h)));
		composite = image_t(canvas_fmt(), vec2D<u16>(w, h));
		bounds = rect<int32_t>{{0, 0}, {w, h}};
		composite_rect(bounds.top_left(), bounds.bottom_right());
	}

	void set_img(image_t img) {
		bounds = rect<int32_t>{{0, 0}, img.size().to<int>()};
		composite = image_t(canvas_fmt(), img.size());
		layers.emplace_back(image_t());
		std::swap(img, layers.back());
		composite_rect(bounds.top_left(), bounds.bottom_right());
	}

	void remove_layer(int layer_id) {
		layers.erase(layers.begin() + layer_id);
		composite_rect(bounds.top_left(), bounds.bottom_right());
	}

	void add_layer() {
		layers.emplace_back(image_t(canvas_fmt(), bounds.size.to<u16>()));
	}

	void reorder_layer(int old_pos, int new_pos) {
		int incr = (old_pos > new_pos) ? -1 : 1;
		for (int i = old_pos; i != new_pos; i += incr) {
			std::swap(layers[i], layers[i + incr]);
		}
		composite_rect(bounds.top_left(), bounds.bottom_right());
	}

	const f32* ptr() { return composite.buf(); }
	rgba at(vec2D<u16> pos) { return layers[active_layer].at(pos); }
	rect<int32_t> get_bounds() { return bounds; }

	commit& get_commit() { return undo_commit; }
	void finalize() { undo_commit = commit(); }

	static image_format canvas_fmt() { return image_format(image_format::buf_t::f32, 4); }
	int active_layer = 0;
private:
	std::vector<image_t> layers;
	image_t composite;
	commit undo_commit;
	rect<int32_t> bounds;

	void composite_pixel(vec2D<int> px) {
		// TODO - this is inefficient
		rgba a = layers[0].at(px.to<u16>());
		for (int i = 1; i < layers.size(); i++) {
			if (a.a() >= 1)
				break;

			rgba b = layers[i].at(px.to<u16>());
			rgba out;
			out[3] = a.a()  + b.a() * (1.0 - a.a());
			for (int j = 0; j < 3; j++) {
				out[j] = ((a[j] * a.a()) + (b[j] * b.a()) * (1.0 - a.a())) / out.a();
			}

			a = out;
		}

		a.gamma_correct(1.0 / 2.2);
		f32* ptr = composite.buf();
		size_t addr = px.x + (px.y * bounds.size.x);
		ptr[addr * 4 + 0] = a.r();
		ptr[addr * 4 + 1] = a.g();
		ptr[addr * 4 + 2] = a.b();
		ptr[addr * 4 + 3] = a.a();
	}

	void composite_rect(vec2D<int> tl, vec2D<int> br) {
		for (int x = tl.x; x < br.x; x++) {
			for (int y = tl.y; y < br.y; y++) {
				composite_pixel({x, y});
			}
		}
	}
};

class undo_stack {
public:
	void push(const commit& c) {
		history.resize(++pos);
		history[pos - 1] = c;
	}

	void undo(layer& l) {
		if (pos != 0)
			swap_commits(l, --pos);
	}

	void redo(layer& l) {
		if (pos != history.size())
			swap_commits(l, pos++);
	}
private:
	void swap_commits(layer& l, size_t idx) {
		l.apply_commit(history[idx]);
		history[idx] = l.get_commit();
		l.finalize();
	}
	std::vector<commit> history;
	size_t pos = 0;
};

struct handle {
	std::array<rgba, 256> palette;
	layer canvas;
	undo_stack history;
	bool tool_active = false;
	uint8_t active_color = 0;
	tool active_tool = tool::null;
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

void set_tool(handle* hnd, tool t) {
	hnd->active_tool = t;
}

bool cursorpress(handle* hnd, int x, int y, unsigned flags) {
	if (true) {
		hnd->tool_active = true;
		switch (hnd->active_tool) {
		case tool::pencil:
			pencil(hnd, hnd->active_color, x, y, x, y);
			break;
		case tool::fill:
			fill(hnd, hnd->active_color, x, y, false);
			break;
		default:
			break;
		}
	} else {
		// cancel the operation
		hnd->tool_active = false;
		hnd->canvas.apply_commit(hnd->canvas.get_commit());
		hnd->canvas.finalize();
	}
}

bool cursordrag(handle* hnd, int x1, int y1, int x2, int y2, unsigned flags) {
	if (hnd->tool_active) {
		switch (hnd->active_tool) {
		case tool::pencil:
			pencil(hnd, hnd->active_color, x1, y1, x2, y2);
			break;
		default:
			break;
		}
	}
}

bool cursorrelease(handle* hnd, unsigned flags) {
	hnd->history.push(hnd->canvas.get_commit());
	hnd->canvas.finalize();
	hnd->tool_active = false;
}

void pencil(handle* hnd, palette_idx c, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	diff d = draw_line(hnd->palette[c], {x1, y1}, {x2, y2}, hnd->canvas.get_bounds());
	hnd->canvas.apply_diff(hnd->canvas.active_layer, d);
}

void fill(handle* hnd, palette_idx c, int x1, int y1, bool global) {
	vec2D<int> size = hnd->canvas.get_bounds().size;
	selection visited;
	selection to_explore;
	diff d;
	rgba fill_color = hnd->palette[c];
	rgba scan_color = hnd->canvas.at({x1, y1});

	to_explore.mark(vec2D<u16>{x1, y1});
	while (!to_explore.empty()) {
		vec2D<u16> p = *(to_explore.begin());
		to_explore.clear(p);
		visited.mark(p);

		add_pixel(fill_color, p.to<int>(), hnd->canvas.get_bounds(), d);

		for (int y = std::max(p.y - 1, 0); y < std::min(p.y + 2, size.y); y++) {
			if (!visited.exists({p.x, y}) && hnd->canvas.at({p.x, y}) == scan_color)
				to_explore.mark(vec2D<u16>{p.x, y});
		}
		for (int x = std::max(p.x - 1, 0); x < std::min(p.x + 2, size.x); x++) {
			if (!visited.exists({x, p.y}) && hnd->canvas.at({x, p.y}) == scan_color)
				to_explore.mark(vec2D<u16>{x, p.y});
		}
	}
	hnd->canvas.apply_diff(hnd->canvas.active_layer, d);
};

void set_active_color(handle* hnd, palette_idx c) { hnd->active_color = c; }
void set_pal_color(handle* hnd, palette_idx c, rgba r) {
	assertion(c < PALETTE_SIZE, "Palette index out of range\n");
	r.gamma_correct(2.2);
	hnd->palette[c] = r; 
}

rgba get_pal_color(handle* hnd, palette_idx c) { 
	assertion(c < PALETTE_SIZE, "Palette index out of range\n");
	rgba r = hnd->palette[c];
	r.gamma_correct(1.0 / 2.2);
	return r;
}

void new_image(handle* hnd, uint16_t w, uint16_t h) { hnd->canvas.new_image(w, h); } 
void load_image(handle* hnd, const char* filename) {
	image_t img = image_t::from_file(filename);
	hnd->canvas.set_img(img.convert_to(layer::canvas_fmt()));
}

void get_imagesize(handle* hnd, uint16_t* w, uint16_t* h) { 
	*w = hnd->canvas.get_bounds().size.x;
	*h = hnd->canvas.get_bounds().size.y;
}
const f32* imagedata(handle* hnd) { return hnd->canvas.ptr(); }


void undo(handle* hnd) { hnd->history.undo(hnd->canvas); }
void redo(handle* hnd) { hnd->history.redo(hnd->canvas); }

void layer_add(handle* hnd) {
	hnd->canvas.add_layer();
}

void layer_remove(handle* hnd, int layer_id) {
	hnd->canvas.remove_layer(layer_id);
}

void layer_select(handle* hnd, int layer_id) {
	hnd->canvas.active_layer = layer_id;
}

void layer_reorder(handle* hnd, int old_pos, int new_pos) {
	hnd->canvas.reorder_layer(old_pos, new_pos);
}
