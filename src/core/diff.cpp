#include "diff.h"
#include <math.h>


void selection::mark(vec2u pos) { ctr.insert_at(pos)->set(ctr.chunk_index(pos)); }
void selection::clear(vec2u pos) { ctr.chunk_at(pos)->clear(ctr.chunk_index(pos)); }
bool selection::exists(vec2u pos) {
	auto* chunk = ctr.chunk_at(pos);
	if (!chunk)
		return false;
	return chunk->get(ctr.chunk_index(pos));
}
bool selection::empty() { return ctr.empty(); }

vec2u selection::iterator::operator*() const {
	vec2u pos = tree_itr.chunk_origin();
	pos.x += (*bitr) % tree_type::chunk_width;
	pos.y += (*bitr) / tree_type::chunk_width;
	return pos;
}

selection::iterator selection::iterator::operator++() {
	if (++bitr == (*tree_itr)->end())
		if (++tree_itr != ctr->ctr.end())
			bitr = (*tree_itr)->begin();
	return *this;
}

selection::iterator::iterator(const selection* ctr, tree_type::iterator tree_itr)
	: ctr(ctr), tree_itr(tree_itr)
{
	if (tree_itr != ctr->ctr.end())
		bitr = (*tree_itr)->begin();
}

void diff::insert(vec2u pos, rgba color) {
	auto* chunk = ctr.insert_at(pos);
	size_t index = ctr.chunk_index(pos);
	if (!chunk->region.get(index)) {
		chunk->region.set(index);
		chunk->colors[index] = color;
	}
}

bool diff::exists(vec2u pos) {
	auto* chunk = ctr.chunk_at(pos);
	if (!chunk)
		return false;
	return chunk->region.get(ctr.chunk_index(pos));
}

std::pair<vec2u, rgba> diff::iterator::operator*() const {
	size_t idx = (*bitr);
	vec2u origin = tree_itr.chunk_origin();
	origin = vec2u(origin.x + (idx % tree_t::chunk_width), origin.y + (idx / tree_t::chunk_width));
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

