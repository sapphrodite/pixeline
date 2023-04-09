#ifndef DIFF_H
#define DIFF_H

#include <common/coordinate_types.h>
#include <common/bitarray.h>
#include <common/image_types.h>
#include <math.h>

#include "spatial_tree.h"

class selection {
	using chunk_type = bitarray<256>;
	using tree_type = spatial_tree<chunk_type>;
public:
	void mark(vec2D<u16> pos);
	void clear(vec2D<u16> pos);
	bool exists(vec2D<u16> pos);

	class iterator {
	public:
		bool operator!=(const iterator& rhs) const = default; 
		bool operator==(const iterator& rhs) const = default;
		vec2D<u16> operator*() const;
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
		size_t run_idx;
		size_t len_pos;
	};

	iterator begin() const { return iterator(this, region.begin(), 0, 0); }
	iterator end() const { return iterator(this, region.end(), runs.size(), 0); }
private:
	u8 palette_idx(rgba color);
	bool same(rgba a, rgba b);

	struct rle {
		u8 pal_idx;
		u8 len;
	};

	std::vector<rgba> colors;
	std::vector<rle> runs;
	chunk_type region;
};



class diff {
	using tree_type = spatial_tree<diff_chunk>;
public:
	void insert(vec2D<u16> pos, rgba color);
	void clear(vec2D<u16> pos);
	bool exists(vec2D<u16> pos);

	
	class iterator {
	public:
		bool operator!=(const iterator& rhs) const = default; 
		bool operator==(const iterator& rhs) const = default;
		std::pair<vec2D<u16>, rgba> operator*() const;
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
