#include "selection.h"
#include <math.h>


void selection::mark(vec2D<u16> pos) {
    auto* chunk = ctr.insert_at(pos);
    chunk->set(ctr.index_into_chunk(pos));
}
    //assertion(n.node_type() == node::type::chunk, "Error accessing node\n");

void selection::clear(vec2D<u16> pos) {
    auto* chunk = ctr.chunk_at(pos);
    chunk->clear(ctr.index_into_chunk(pos));
}

bool selection::exists(vec2D<u16> pos) {
    auto* chunk = ctr.chunk_at(pos);
    if (!chunk)
        return false;
    return chunk->get(ctr.index_into_chunk(pos));
}

vec2D<u16> selection::iterator::operator*() const {
	vec2D<u16> pos = tree_itr.chunk_origin();
	return vec2D<u16>(pos.x + ((*bitr) % tree_type::chunk_width), pos.y + ((*bitr) / tree_type::chunk_width));
}

selection::iterator selection::iterator::operator++() {
    if (++bitr == (*tree_itr)->end())
        if (++tree_itr != ctr->ctr.end())
            bitr = (*tree_itr)->begin();
    return *this;
}

selection::iterator::iterator(const selection* ctr, tree_type::iterator tree_itr) : ctr(ctr), tree_itr(tree_itr) {
    if (tree_itr != ctr->ctr.end())
        bitr = (*tree_itr)->begin();
}







void diff_chunk::insert(size_t index, rgba color) {
    region.set(index);
    size_t idx = region.popcnt_before(index);

    // if there's no runs (idx = 0?), just append some shit easy
    u8 pal_idx = palette_idx(color);
    size_t pos = 0;
    for (size_t i = 0; i < runs.size(); i++) {
        bool same_color = pal_idx == runs[i].pal_idx;

        // append condition -
        if (idx >= pos && idx <= pos + runs[i].len && same_color) {
            runs[i].len++;
            return;
        }

        // split condition - index is within this run, but has different color
        // note - bottom bound is inclusive, because if the previous iteration could've appended, iteration would've broken
        if (pos + runs[i].len > idx && !same_color) {
            // if we're inserting between runs, it's not actually a split
            if (idx == pos) {
                runs.insert(runs.begin() + i, rle{pal_idx, 1});
                return;
            }
            u8 left_size = idx - pos;
            u8 right_size = runs[i].len - left_size;
            runs[i].len = left_size;
            runs.insert(runs.begin() + i + 1, rle{pal_idx, 1});
            runs.insert(runs.begin() + i + 1, rle{runs[i].pal_idx, right_size});
            return;
        }

        pos += runs[i].len;
    }

    // fallback
    runs.emplace_back(rle{pal_idx, 1});
}

bool diff_chunk::exists(size_t index) { return region.get(index); }

std::pair<size_t, rgba> diff_chunk::iterator::operator*() const {
    return std::make_pair(*bitr, ctr->colors[ctr->runs[run_idx].pal_idx]);
}

diff_chunk::iterator diff_chunk::iterator::operator++() {
    if (++len_pos == ctr->runs[run_idx].len) {
        run_idx++;
        len_pos = 0;
    }
    ++bitr;
	return *this; 
}

diff_chunk::iterator::iterator(const diff_chunk* ctr, chunk_type::iterator bitr, size_t run_idx, size_t len_pos)
        : ctr(ctr), bitr(bitr), run_idx(run_idx), len_pos(len_pos) {}

u8 diff_chunk::palette_idx(rgba color) {
    for (size_t i = 0; i < colors.size(); i++) {
        if (same(colors[i], color))
            return i;
    }
    
    // if no match found
    colors.emplace_back(color);
    return colors.size() - 1;
}

bool diff_chunk::same(rgba a, rgba b) {
    float err = pow(10, -6);
    return (abs(a.r - b.r) < err && abs(a.g - b.g) < err
            && abs(a.b - b.b) < err && abs(a.a - b.a) < err);
}




void diff::insert(vec2D<u16> pos, rgba color) {
    auto* chunk = ctr.insert_at(pos);
    chunk->insert(ctr.index_into_chunk(pos), color);
}

void diff::clear(vec2D<u16> pos) {
//    auto* chunk = ctr.chunk_at(pos);
//    chunk->clear(ctr.index_into_chunk(pos));
}

bool diff::exists(vec2D<u16> pos) {
    auto* chunk = ctr.chunk_at(pos);
    if (!chunk)
        return false;
    return chunk->exists(ctr.index_into_chunk(pos));
}

std::pair<vec2D<u16>, rgba> diff::iterator::operator*() const {
    size_t idx = (*bitr).first;
    vec2D<u16> origin = tree_itr.chunk_origin();
    origin = vec2D<u16>(origin.x + (idx % tree_type::chunk_width), origin.y + (idx / tree_type::chunk_width));
    return std::make_pair(origin, (*bitr).second);
}

diff::iterator diff::iterator::operator++() {
	if (++bitr == (*tree_itr)->end())
		if (++tree_itr != ctr->ctr.end()) 
			bitr = (*tree_itr)->begin();
	return *this;
}

diff::iterator::iterator(const diff* ctr, tree_type::iterator tree_itr) : ctr(ctr), tree_itr(tree_itr) {
    if (tree_itr != ctr->ctr.end())
        bitr = (*tree_itr)->begin();
}

