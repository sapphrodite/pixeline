#ifndef DIFF_H
#define DIFF_H

#include "image_types.h"
#include "spatial_tree.h"
#include <include/bitarray.h>
#include <include/coordinate_types.h>

class selection {
	using chunk_type = bitarray<256>;
	using tree_type = spatial_tree<chunk_type>;
public:
	void mark(vec2u pos);
	void clear(vec2u pos);
	bool exists(vec2u pos);
	bool empty();

	class iterator {
	public:
		bool operator!=(const iterator& rhs) const = default;
		bool operator==(const iterator& rhs) const = default;
		vec2u operator*() const;
		iterator operator++();
	private:
		friend class selection;
		iterator(const selection* ctr, tree_type::iterator tree_itr);

		const selection* ctr;
		tree_type::iterator tree_itr = tree_type().end();
		chunk_type::iterator bitr = chunk_type().end();
	};

	iterator begin() const { return iterator(this, ctr.begin()); }
	iterator end() const { return iterator(this, ctr.end()); }
private:
	tree_type ctr;
};

struct diff_chunk {
	std::array<rgba, 256> colors;
	bitarray<256> region;
	size_t size() const { return region.size(); }
	constexpr size_t capacity() const { return 256; }
};

class diff {
	using tree_t = spatial_tree<diff_chunk>;
public:
	void insert(vec2u pos, rgba color);
	void clear(vec2u pos);
	bool exists(vec2u pos);
	
	class iterator {
	public:
		bool operator!=(const iterator& rhs) const = default; 
		bool operator==(const iterator& rhs) const = default;
		std::pair<vec2u, rgba> operator*() const;
		iterator operator++();
	private:
		friend class diff;
		iterator(const diff* ctr, spatial_tree<diff_chunk>::iterator tree_itr);

		const diff* ctr;
		spatial_tree<diff_chunk>::iterator tree_itr;
		bitarray<256>::iterator bitr = bitarray<256>().end();
	};

	iterator begin() const { return iterator(this, ctr.begin()); }
	iterator end() const { return iterator(this, ctr.end()); }
private:
	spatial_tree<diff_chunk> ctr;
};

#endif //DIFF_H
