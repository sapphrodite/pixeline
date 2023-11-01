#include "image_types.h"

#include <libpng/png.h>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <cstring>

#include <cmath>
#include <variant>
#include <algorithm>


image_format::image_format(buf_t buftype, int num_channels) {
	assert(num_channels > 0);
	data = (num_channels - 1) & 0x3;
	_buftype = buftype;
}
int image_format::num_channels() { return (data & 0x3) + 1; }
size_t image_format::size_of(int x, int y) {
	int len = x * y * num_channels();
	#define BUFTYPE_SIZE(T) \
	if ( image_format::buf_t::T == _buftype ) \
		len *= sizeof(T);
	BUFTYPES(BUFTYPE_SIZE)
	return len;
}


struct image_t::pimpl {
public:

	#define INIT_CALL(T) \
		if (fmt.buftype() == image_format::buf_t::T) { \
			data = std::unique_ptr<T[]>(new T[len]); \
			auto* ptr = buf_at<T>(0); \
			for (int i = 0; i < len; i++) \
				ptr[i] = 1; \
		} else

	#define RET_CALL(T) \
		if (fmt.buftype() == image_format::buf_t::T) \
			return (void*) buf_at<T>(idx); \
		else

	vec2u size;
	image_format fmt;

	pimpl(image_format fmt_in, vec2u size_in) : size(size_in), fmt(fmt_in) {
		size_t len = fmt.size_of(size.x, size.y);
		BUFTYPES(INIT_CALL)
			throw std::logic_error("Unknown buftype");
	}

	template <typename T>
	T* buf_at(size_t idx) {
		return std::get<std::unique_ptr<T[]>>(data).get() + idx;
	}

	void* buf() { return buf_at(0); }
	void* buf_at(size_t idx) {
		BUFTYPES(RET_CALL)
			throw std::logic_error("Unknown buftype");
	}

private:
	std::variant<std::unique_ptr<u8[]>, std::unique_ptr<f32[]>, std::unique_ptr<u16[]>> data;
};

image_t::~image_t() = default;
image_t::image_t() = default;
image_t::image_t(image_t&& other) = default;
image_t& image_t::operator=(image_t&& other) = default;
image_t::image_t(image_format fmt, vec2u size) {
	data = std::make_unique<pimpl>(pimpl(fmt, size));
}

template <typename T>
void copy_itof(f32** dst, image_format dstfmt, T** src, image_format srcfmt) {
	int j = 0;
	for (; j < 3; j++) {
		int src_idx = srcfmt.num_channels() == 1 ? 0 : j;
		(*dst)[j] = (f32) (*src)[src_idx] / (float) std::numeric_limits<T>::max();
	}
	for (int i = j; i < dstfmt.num_channels(); i++)
		(*dst)[i] = 1;
	*src += srcfmt.num_channels();
	*dst += dstfmt.num_channels();
}


void copy_pixel(void** dst, image_format dstfmt, void** src, image_format srcfmt) {
	if (dstfmt.is(image_format::buf_t::f32) && srcfmt.is(image_format::buf_t::u8)) {
		copy_itof((f32**) dst, dstfmt, (u8**) src, srcfmt);
	} else if (dstfmt.is(image_format::buf_t::f32) && srcfmt.is(image_format::buf_t::u16)) {
		// 16-bit PNGs are big endian. TODO - proper endian test/swap
		for (int i = 0; i < srcfmt.num_channels(); i++) {
			u16* ptr = (u16*) (*src);
			ptr[i] = (ptr[i] >> 8) | (ptr[i] << 8);
		}
		copy_itof((f32**) dst, dstfmt, (u16**) src, srcfmt);
	} else {
		throw std::logic_error("Unhandled conversion operation");
	}
}

image_t image_t::from_file(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	assert(fp);
	png_struct* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info* info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);

	// TODO - get an actual system value, put it in the API
	double gamma;
	if (png_get_gAMA(png_ptr, info_ptr, &gamma))
		png_set_gamma(png_ptr, 1.0, gamma);
	else
		png_set_gamma(png_ptr, 1.0, 1.0 / 2.2);


	png_read_update_info(png_ptr, info_ptr);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	int color_t = png_get_color_type(png_ptr, info_ptr);
	int bpp = png_get_bit_depth(png_ptr, info_ptr);
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	int nc = 0;
	nc += color_t & PNG_FORMAT_FLAG_ALPHA ? 1 : 0;
	nc += color_t & PNG_FORMAT_FLAG_COLOR ? 3 : 1;

	image_format::buf_t buftype = bpp / nc == 16 ? image_format::buf_t::u16 : image_format::buf_t::u8;

	image_format fmt(buftype, nc);
	image_t img(fmt, vec2u(width, height));

	// What in the actual fuck is this? thanks libpng
	std::vector<png_byte*> ptr_storage(height);
	for (int y = 0; y < height; y++)
		ptr_storage[y] = ((uint8_t*) img.data.get()->buf()) + (rowbytes * y);
	png_read_image(png_ptr, ptr_storage.data());

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);


	return img;
}

image_t image_t::convert_to(image_format dstfmt) {
	image_t dst(dstfmt, data.get()->size);

	void* dstptr = dst.data.get()->buf();
	void* srcptr = data.get()->buf();
	for (int i = 0; i < size().x * size().y; i++)
		copy_pixel(&dstptr, dstfmt, &srcptr, data.get()->fmt);

	return dst;
}

rgba image_t::at(vec2u pos) { return at(pos.x + (pos.y * size().x)); }
rgba image_t::at(size_t idx) {
	if (data.get()->fmt.is(image_format::buf_t::f32) && data.get()->fmt.num_channels() >= 3) {
		int nc = data.get()->fmt.num_channels();
		idx *= nc;
		f32* buf = data.get()->buf_at<f32>(0);
		rgba ret{1, 1, 1, 1};
		for (int i = 0; i < nc; i++)
			ret[i] = buf[idx + i];
		return ret;
	} else {
		throw std::logic_error("Unsupported conversion");
	}
}

f32* image_t::buf() { return data.get()->buf_at<f32>(0); }

vec2u image_t::size() { return data.get()->size; }


rgba rgba::from(const hsv& a) {
	hsv in = a;
	in[1] /= 100.0f;
	in[2] /= 100.0f;

	const f32 z = in.v() * in.s();
	const f32 x = z * (1 - fabs(fmod(in.h() / 60.0, 2) - 1));
	const f32 m = in.v() - z;

	f32 rp = 0, gp = 0, bp = 0;

	gp = in.h() >= 60 ? z : x;
	bp = in.h() < 120 ? 0 : x;

	if ((in.h() >= 0) && (in.h() < 60)) {
		rp = z;
		gp = x;
		bp = 0;
	} else if (in.h() >= 60 && in.h() < 120) {
		rp = x;
		gp = z;
		bp = 0;
	} else if (in.h() >= 120 && in.h() < 180) {
		rp = 0;
		gp = z;
		bp = x;
	} else if (in.h() >= 180 && in.h() < 240) {
		rp = 0;
		gp = x;
		bp = z;
	} else if (in.h() >= 240 && in.h() < 300) {
		rp = x;
		gp = 0;
		bp = z;
	} else if (in.h() >= 300 && in.h() <= 360) {
		rp = z;
		gp = 0;
		bp = x;
	}

	return rgba((rp + m), (gp + m), (bp + m), 1);
}

hsv hsv::from(const rgba& in) {
	f32 rp = in.r();
	f32 gp = in.g();
	f32 bp = in.b();

	f32 cmax = std::max({rp, gp, bp});
	f32 cmin = std::min({rp, gp, bp});
	f32 delta = cmax - cmin;

	hsv ret(0, 0, 0);
	if (cmax == rp) {
		ret[0] = 60 * fmod((gp - bp) / delta, 6) + 360;
	} else if (cmax == gp) {
		ret[0] = 60 * ((bp - rp) / delta) + 120;
	} else if (cmax == bp) {
		ret[0] = 60 * ((rp - gp) / delta) + 240;
	}

	ret[1] = (1.0f / (cmax / delta));
	ret[2] = cmax;
	return hsv(fmod(ret.h(), 360), ret.s() * 100, ret.v() * 100);
}
