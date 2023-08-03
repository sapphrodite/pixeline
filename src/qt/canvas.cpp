#include <QPainter>
#include <QMouseEvent>
#include <core/image_types.h>
#include "canvas.h"

canvas::canvas(handle* hnd) : hnd(hnd) {
	_zoom = 5;
}

void canvas::paintEvent(QPaintEvent*) {
	::rect<u16> box = get_canvas_box();
	QPainter painter(this);

	// draw outline for reference
	painter.setPen(QPen());
	painter.drawRect(QRect(box.origin.x - 1, box.origin.y - 1 , box.size.x + 1, box.size.y + 1));
	painter.setPen(Qt::NoPen);

	const f32* data = imagedata(hnd);
	for (int y = 0; y < size().y; y++) {
		for (int x = 0; x < size().x; x++) {
			const f32* p = data + ((x + (y * size().x)) * 4);
			painter.setBrush(QBrush(QColor(p[0] * 255, p[1] * 255, p[2] * 255)));
			painter.drawRect(QRect(box.origin.x + x * _zoom, box.origin.y + y * _zoom,  _zoom, _zoom));
		}
	}
}

void canvas::mouseReleaseEvent(QMouseEvent* e) {
	cursorrelease(hnd, 0);
}

void canvas::mousePressEvent(QMouseEvent* e) {
	vec2D<i32> new_pos = imagespace_coords(e);
	cursorpress(hnd, new_pos.x, new_pos.y, 0);
	last_pos = new_pos;
	repaint();
}

void canvas::mouseMoveEvent(QMouseEvent* e) {
	vec2D<i32> new_pos = imagespace_coords(e);
	cursordrag(hnd, last_pos.x, last_pos.y, new_pos.x, new_pos.y, 0);
	last_pos = new_pos;
	repaint();
}

vec2D<u16> canvas::size() { 
	vec2D<u16> retval;
	get_imagesize(hnd, &retval.x, &retval.y);
	return retval;	
}

rect<u16> canvas::get_canvas_box() {
	vec2D<u16> boxsize(size().x * _zoom, size().y * _zoom);
	vec2D<u16> origin((width() - boxsize.x) / 2,  (height() - boxsize.y) / 2);
	return ::rect<u16>(origin, boxsize);
}

vec2D<i32> canvas::imagespace_coords(QMouseEvent* e) {
	::rect<u16> canvas_box = get_canvas_box();
	vec2D<i32> p(e->x() - canvas_box.origin.x, e->y() - canvas_box.origin.y);
	return vec2D<i32>(p.x / _zoom, p.y / _zoom);
}
