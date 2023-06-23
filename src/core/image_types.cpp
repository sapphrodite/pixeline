#include "image_types.h"
#include <cmath>
#include <algorithm>

f32& hsv::operator[](size_t idx) {
	assert(idx < 4);
	return data[idx];
}

f32& rgba::operator[](size_t idx) {
	assert(idx < 5);
	return data[idx];
}

rgba to_rgb(hsv in) {
	in.s() /= 100.0f;
	in.v() /= 100.0f;

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

hsv to_hsv(rgba in) {
	f32 rp = in.r();
	f32 gp = in.g();
	f32 bp = in.b();

	f32 cmax = std::max({rp, gp, bp});
	f32 cmin = std::min({rp, gp, bp});
	f32 delta = cmax - cmin;

	hsv ret(0, 0, 0);
	if (cmax == rp) {
		ret.h() = 60 * fmod((gp - bp) / delta, 6) + 360;
	} else if (cmax == gp) {
		ret.h() = 60 * ((bp - rp) / delta) + 120;
	} else if (cmax == bp) {
		ret.h() = 60 * ((rp - gp) / delta) + 240;
	}

	ret.s() = (1.0f / (cmax / delta));
	ret.v() = cmax;
	return hsv(fmod(ret.h(), 360), ret.s() * 100, ret.v() * 100);
}

image_format::image_format(bool is_float, int num_channels) {
	assert(num_channels > 0);
	data = (num_channels - 1) & 0x3;
	data |= (is_float ? 1 : 0) << 2;
}

size_t image_format::size_of(int x, int y) { return x * y * num_channels(); }

#include <libpng/png.h>
#include <cstdio>
#include <vector>
#include <stdexcept>

struct png_reader::pimpl {
	FILE* fp = nullptr;
	png_struct* png_ptr = nullptr;
	png_info* info_ptr = nullptr;
};

png_reader::png_reader(const char* filename) {
	data = new pimpl;
	data->fp = fopen(filename, "rb");
	assert(data->fp);
	char header[8]; // 8 is the maximum size that can be checked
	fread(header, 1, 8, data->fp);  // This advances the read pointer - do not remove

	data->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	data->info_ptr = png_create_info_struct(data->png_ptr);

	png_init_io(data->png_ptr, data->fp);
	png_set_sig_bytes(data->png_ptr, 8);
	png_read_info(data->png_ptr, data->info_ptr);
	png_read_update_info(data->png_ptr, data->info_ptr);
}

png_reader::~png_reader() {
	png_destroy_read_struct(&data->png_ptr, &data->info_ptr, NULL);
	fclose(data->fp);
	delete data;
}

image_format png_reader::get_fmt() { return image_format(false, 3); }
size_t png_reader::width() { return png_get_image_width(data->png_ptr, data->info_ptr); }
size_t png_reader::height() { return png_get_image_height(data->png_ptr, data->info_ptr); }
size_t png_reader::lenbytes() { return height() * rowbytes(); }
size_t png_reader::read_image(uint8_t* buf) {
	// What in the actual fuck is this? thanks libpng
	std::vector<png_byte*> ptr_storage(height());
	for (int y = 0; y < height(); y++) {
		ptr_storage[y] = buf + (rowbytes() * y);
	}
	png_read_image(data->png_ptr, ptr_storage.data());
	return lenbytes();
}
size_t png_reader::rowbytes() { return png_get_rowbytes(data->png_ptr, data->info_ptr); }




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

void convert_image_fmt(void* dst, image_format dstfmt, void* src, image_format srcfmt,
		size_t w, size_t h) {
	// dest num channels can't be lower than src, not easily

	void* dstptr = dst;
	void* srcptr = src;
	for (int i = 0; i < w * h; i++)
		copy_pixel(&dstptr, dstfmt, &srcptr, srcfmt);
}
