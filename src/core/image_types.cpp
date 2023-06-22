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
