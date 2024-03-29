#ifndef DIFF_H
#define DIFF_H

#include <common/coordinate_types.h>
#include <common/bitarray.h>
#include <core/image_types.h>
#include <math.h>

#include "spatial_tree.h"

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


class diff_chunk {
	using chunk_type = bitarray<256>;
public:
	void insert(size_t index, rgba color);
	// bool remove(size_t index, rgba color)
	bool exists(size_t index);

	size_t size() const { return region.size(); }
	constexpr size_t capacity() const { return region.capacity(); }

	class iterator {
	public:
		bool operator!=(const iterator& rhs) const { return bitr != rhs.bitr; }
		bool operator==(const iterator& rhs) const { return bitr == rhs.bitr; }
		std::pair<size_t, rgba> operator*() const;
		iterator operator++();
	private:
		friend class diff_chunk;
		iterator(const diff_chunk* ctr, chunk_type::iterator bitr, size_t run_idx, size_t len_pos);

		const diff_chunk* ctr;
		chunk_type::iterator bitr;
	};

	iterator begin() const { return iterator(this, region.begin(), 0, 0); }
	iterator end() const { return iterator(this, region.end(), 0, 0); }
private:
	std::array<rgba, 256> colors;
	chunk_type region;
};



class diff {
	using tree_type = spatial_tree<diff_chunk>;
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
		spatial_tree<diff_chunk>::iterator tree_itr = spatial_tree<diff_chunk>().end();
		diff_chunk::iterator bitr = diff_chunk().end();
	};

	iterator begin() const { return iterator(this, ctr.begin()); }
	iterator end() const { return iterator(this, ctr.end()); }
private:
	spatial_tree<diff_chunk> ctr;
};


#endif //DIFF_H
