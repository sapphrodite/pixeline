#include <QPainter>
#include <QMouseEvent>
#include <core/image_types.h>
#include "canvas.h"

canvas::canvas(handle* hnd) : hnd(hnd) {
	_zoom = 5;
}

void canvas::paintEvent(QPaintEvent*) {
	::rect box = get_canvas_box();
	QPainter painter(this);

	// draw outline for reference
	painter.setPen(QPen());
	painter.drawRect(QRect(box.origin.x - 1, box.origin.y - 1 , box.size.x + 1, box.size.y + 1));
	painter.setPen(Qt::NoPen);

	const f32* data = imagedata(hnd);
	vec2u size = get_imagesize(hnd);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			const f32* p = data + ((x + (y * size.x)) * 4);
			painter.setBrush(QBrush(QColor(p[0] * 255, p[1] * 255, p[2] * 255)));
			painter.drawRect(QRect(box.origin.x + x * _zoom, box.origin.y + y * _zoom,  _zoom, _zoom));
		}
	}
}

void canvas::mouseReleaseEvent(QMouseEvent* e) {
	cursorrelease(hnd, 0);
}

void canvas::mousePressEvent(QMouseEvent* e) {
	vec2i new_pos = imagespace_coords(e);
	cursorpress(hnd, new_pos, 0);
	last_pos = new_pos;
	repaint();
}

void canvas::mouseMoveEvent(QMouseEvent* e) {
	vec2i new_pos = imagespace_coords(e);
	cursordrag(hnd, last_pos, new_pos, 0);
	last_pos = new_pos;
	repaint();
}

rect canvas::get_canvas_box() {
	vec2u boxsize = get_imagesize(hnd) * vec2u{_zoom, _zoom};
	vec2i origin((width() - boxsize.x) / 2,  (height() - boxsize.y) / 2);
	return ::rect(origin, boxsize.to<int>());
}

vec2i canvas::imagespace_coords(QMouseEvent* e) {
	::rect canvas_box = get_canvas_box();
	vec2i p(e->x() - canvas_box.origin.x, e->y() - canvas_box.origin.y);
	return vec2i(p.x / _zoom, p.y / _zoom);
}
