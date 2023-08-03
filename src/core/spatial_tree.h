#ifndef SPATIAL_TREE_H
#define SPATIAL_TREE_H

#include <common/integral_types.h>
#include <common/coordinate_types.h>
#include <common/bit.h>
#include <common/assertion.h>

#include <math.h>
#include <string.h>

template <typename T>
class spatial_tree {
	// how many chunks are covered with each child node access, or "step" in a traversal path.
	// e.g. a step width of 2 means the X and Y coordinates have 2 bits each, resulting in an area of 16 (2^(2+2)) chunks.
	constexpr static auto step_width = 2;

	// larger step values require more children per node; 
	constexpr static auto node_width = 1 << step_width;
	constexpr static auto node_size = node_width * node_width; 

	using path_record = bitstring<u32>;
	using node = uint32_t;
	constexpr static node null_node = -1;
public:
	// 2^(floor(log2(capacity - 1)) / 2)
	constexpr static auto chunk_width = 1 << ((std::bit_width(T().capacity()) - 1) / 2);
	static_assert(T().capacity() == (chunk_width * chunk_width), "Chunk width not even power of two, wasting space."); 

	class iterator {
	public:
		bool operator==(const iterator& rhs) const = default;
		bool operator!=(const iterator& rhs) const = default;
		const T* operator*() const { return &_ctr->_chunks[chunk_idx]; }

		vec2D<u16> chunk_origin() const { return _ctr->chunk_origin(_path); }
		iterator operator++() {
			_path = bitstring<u32>(_path.value() + 1);
			load_chunk();
			return *this;
		}

	private:
		friend class spatial_tree;

		path_record _path = path_record(0);
		const spatial_tree<T>* _ctr;
		size_t chunk_idx = 0;

		iterator(const spatial_tree* ctr) : _ctr(ctr) {
			if (_ctr != nullptr)
				load_chunk();
		}

		node find_next_chunk(node current, size_t depth) {
			if (current == null_node || pte() || (depth == _ctr->_depth && !is_valid_chunk(current)))
				return null_node; 
			if (depth == _ctr->_depth)
				return current;

			for (int i = _ctr->path_getstep(_path, depth); i < node_size; i++) {
				_ctr->path_writestep(_path, depth, i);
				node next = find_next_chunk(_ctr->_child(current, i), depth + 1);
				if (next != null_node)
					return next;
			}

			_ctr->path_writestep(_path, depth, 0);
			return null_node;
		}

		void load_chunk() {
			node next = find_next_chunk(_ctr->_root, 0);
			if (next != null_node) {
				chunk_idx = next;
			} else {
	*this = iterator(nullptr);
			}
		}

		bool pte() { return _path.value() >> (_ctr->_depth * (2 * step_width)); }
		bool is_valid_chunk(node ch) {  return ch < _ctr->_chunks.size() && _ctr->_chunks[ch].size() != 0; }
	};


	size_t index_into_chunk(vec2D<u16> pos) { return (pos.x % chunk_width) + ((pos.y % chunk_width) * chunk_width); }

	bool inbounds(vec2D<u16> pos) {
		// how many nodes long is the tree? 
		uint16_t total_width = chunk_width * (1 << (step_width * _depth));
		return (pos.x >= origin.x && pos.y >= origin.y &&
			pos.x < origin.x + total_width && pos.y < origin.y + total_width);
	}

	T* insert_at(vec2D<u16> pos)  {
		while (!inbounds(pos)) {
			_depth++;
			uint16_t total_width = chunk_width * (1 << (step_width * _depth));
			node new_root = _add_node_at_depth(0);

			if (_depth != 0) {
				unsigned childidx = path_getstep(path_to(origin), 0);
				 _child(new_root, childidx) = _root;
			} else {
	origin = pos;
			}

			origin = vec2D<u16>(origin.x & ~(total_width - 1), origin.y & ~(total_width - 1));
			_root = new_root;
		}

		pos = vec2D<u16>(pos.x - origin.x, pos.y - origin.y);

		path_record path = path_to(pos);
		node current = _root;
		for (int i = 0; i < _depth; i++) {
			node next = _child(current, path_getstep(path, i));
			if (next == null_node) {
				next = _add_node_at_depth(i + 1);
				 _child(current, path_getstep(path, i)) = next;

			}
			current = next;
		}

		return &_chunks[current];
	}

	T* chunk_at(vec2D<u16> pos) {
		if (!inbounds(pos))
			return nullptr;

		path_record path = path_to(pos);
		node current = _root;
		for (int i = 0; i < _depth; i++) {
			current = _child(current, path_getstep(path, i));
			if (current == null_node)
				return nullptr;
		}
		return &_chunks[current];
	}

	bool empty() const { return begin() == end(); }
	iterator begin() const { return iterator(this); }
	iterator end() const { return iterator(nullptr); }
private:
	node& _child(node parent, size_t child_idx) { return _subnodes[parent][child_idx]; }
	node _child(node parent, size_t child_idx) const { return _subnodes[parent][child_idx]; }

	node _add_node(auto& nodelist) {
		node n(nodelist.size());
		nodelist.emplace_back();
		assertion(n == nodelist.size() - 1, "Index was improperly written to node");
		return n;
	}

	node _add_node_at_depth(size_t depth) {
		if (depth < _depth) {
			node n = _add_node(_subnodes);
			memset(_subnodes[n].data(), 0xFF, node_size * sizeof(node));
			return n;
		} else {
			return _add_node(_chunks);
		}
	}

	void path_writestep(path_record& p, size_t lvl, size_t step) const { p.write(mask::packed(_depth - 1 - lvl, 2 * step_width), step); }
	size_t path_getstep(path_record p, size_t lvl) const { return p.get(mask::packed(_depth - 1 - lvl, 2 * step_width)); }

	path_record path_to(vec2D<u16> dest) const {
		dest = vec2D<u16>(dest.x / chunk_width, dest.y / chunk_width);
		path_record p = path_record(0);
		size_t bmask = node_width - 1;
		for (int i = 0; i < _depth; i++) {
			path_writestep(p, _depth - 1 - i, (dest.x & bmask) | ((dest.y & bmask) << step_width));
			dest = vec2D<u16>(dest.x >> step_width, dest.y >> step_width);
		}
		return p;
	}

	vec2D<u16> chunk_origin(path_record p) const {
		vec2D<u16> pos = origin;
		size_t shift_coeff = chunk_width;
		size_t bmask = node_width - 1; 
		for (int i = 0; i < _depth; i++) {
			pos.x += (path_getstep(p, _depth - 1 - i) & bmask) * shift_coeff;
			pos.y += (path_getstep(p, _depth - 1 - i) >> step_width) * shift_coeff;
			shift_coeff <<= step_width;
		}
		return pos;
	}

	std::vector<std::array<node, node_size>> _subnodes;
	std::vector<T> _chunks;
	node _root = node(-1);
	path_record recent_path;
	
	vec2D<uint16_t> origin = vec2D<u16>(-1, -1);
	size_t _depth = -1;
};


#endif //SPATIAL_TREE_H
