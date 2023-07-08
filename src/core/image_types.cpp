#include "image_types.h"

#include <libpng/png.h>
#include <cstdio>
#include <vector>
#include <stdexcept>

#include <cmath>
#include <variant>
#include <algorithm>


image_format::image_format(bool is_float, int num_channels) {
	assert(num_channels > 0);
	data = (num_channels - 1) & 0x3;
	data |= (is_float ? 1 : 0) << 2;
}
bool image_format::is_float() { return data >> 2; }
int image_format::num_channels() { return (data & 0x3) + 1; }
size_t image_format::size_of(int x, int y) { return x * y * num_channels(); }


struct image_t::pimpl {
	vec2D<u16> size;
	std::variant<u8*, f32*> data = (u8*) 0x0;
	image_format fmt;
};

image_t::~image_t() = default;
image_t::image_t() = default;
image_t::image_t(image_t&& other) = default;
image_t& image_t::operator=(image_t&& other) = default;
image_t::image_t(image_format fmt, vec2D<u16> size) {
	data = std::make_unique<image_t::pimpl>();
	data.get()->fmt = fmt;
	data.get()->size = size;
	if (fmt.is_float()) {
		f32* buf = new f32[fmt.size_of(size.x, size.y)];
		data.get()->data = buf;
	} else {
		u8* buf = new u8[fmt.size_of(size.x, size.y)];
		data.get()->data = buf;
	}
}

void copy_pixel_itof(f32** dst, image_format dstfmt, u8** src, image_format srcfmt) {
	int j = 0;
	for (; j < srcfmt.num_channels(); j++)
		(*dst)[j] = (f32) (*src)[j] / 255.0;
	for (int i = j; i < dstfmt.num_channels(); i++)
		(*dst)[i] = 1;
	*src += srcfmt.num_channels();
	*dst += dstfmt.num_channels();
}

void copy_pixel(void** dst, image_format dstfmt, void** src, image_format srcfmt) {
	if (dstfmt.is_float() && !srcfmt.is_float()) {
		copy_pixel_itof((f32**) dst, dstfmt, (u8**) src, srcfmt);
	} else {
		throw std::logic_error("Unhandled conversion opertation");
	}
}

image_t image_t::from_file(const char* filename) {
	FILE* fp = nullptr;
	png_struct* png_ptr = nullptr;
	png_info* info_ptr = nullptr;

	fp = fopen(filename, "rb");
	assert(fp);
	char header[8]; // 8 is the maximum size that can be checked
	fread(header, 1, 8, fp);  // This advances the read pointer - do not remove

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);
	png_read_update_info(png_ptr, info_ptr);


	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	image_format fmt(false, 3);

	image_t img(fmt, vec2D<u16>(width, height));

	// What in the actual fuck is this? thanks libpng
	std::vector<png_byte*> ptr_storage(height);
	for (int y = 0; y < height; y++)
		ptr_storage[y] = (uint8_t*) img.buf() + (rowbytes * y);
	png_read_image(png_ptr, ptr_storage.data());

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	return img;
}

image_t image_t::convert_to(image_format dstfmt) {
	image_t dst(dstfmt, data.get()->size);

	// dest num channels can't be lower than src, not easily

	void* dstptr = dst.buf();
	void* srcptr = this->buf();
	for (int i = 0; i < size().x * size().y; i++)
		copy_pixel(&dstptr, dst.data.get()->fmt, &srcptr, data.get()->fmt);

	return dst;
}

rgba image_t::at(vec2D<u16> position) {
	return at(position.x + (position.y * size().x));
}

rgba image_t::at(size_t idx) {
	if (data.get()->fmt.is_float()) {
		f32* buf = std::get<f32*>(data.get()->data);
		return rgba{buf[idx], buf[idx + 1], buf[idx + 2], buf[idx + 3]};
	} else {
		throw std::logic_error("oops!");
	}
}

vec2D<u16> image_t::size() { return data.get()->size; }
void* image_t::buf() {
	if (data.get()->fmt.is_float())
		return (void*) std::get<float*>(data.get()->data);
	else
		return (void*) std::get<u8*>(data.get()->data);
}



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
