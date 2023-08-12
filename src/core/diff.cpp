#include "diff.h"
#include <math.h>


void diff::insert(vec2D<u16> pos, rgba color) {
	auto* chunk = ctr.insert_at(pos);
	chunk->region.set(ctr.chunk_index(pos));
	chunk->colors[ctr.chunk_index(pos)] = color;
}

bool diff::exists(vec2D<u16> pos) {
	auto* chunk = ctr.chunk_at(pos);
	if (!chunk)
		return false;
	return chunk->region.get(ctr.chunk_index(pos));
}

std::pair<vec2D<u16>, rgba> diff::iterator::operator*() const {
	size_t idx = (*bitr);
	vec2D<u16> origin = tree_itr.chunk_origin();
	origin = vec2D<u16>(origin.x + (idx % tree_t::chunk_width), origin.y + (idx / tree_t::chunk_width));
	return std::make_pair(origin, (*tree_itr)->colors[(*bitr)]);
}

diff::iterator diff::iterator::operator++() {
	if (++bitr == (*tree_itr)->region.end())
		if (++tree_itr != ctr->ctr.end()) 
			bitr = (*tree_itr)->region.begin();
	return *this;
}

diff::iterator::iterator(const diff* ctr, tree_t::iterator tree_itr) : ctr(ctr), tree_itr(tree_itr) {
	if (tree_itr != ctr->ctr.end())
		bitr = (*tree_itr)->region.begin();
}

