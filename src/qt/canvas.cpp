#include "canvas.h"
#include <core/api.h>
#include <QMouseEvent>
#include <QPainter>

canvas::canvas(handle* hnd) : hnd(hnd) {
	zoom = 5;
}

void canvas::paintEvent(QPaintEvent*) {
	::rect<u16> box = viewport();
	QPainter painter(this);

	// draw outline for reference
	painter.setPen(QPen());
	painter.drawRect(QRect(box.origin.x - 1, box.origin.y - 1 , box.size.x + 1, box.size.y + 1));
	painter.setPen(Qt::NoPen);

	vec2D<u16> size;
	image_size(hnd, &size.x, &size.y);
	const f32* data = image_data(hnd);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			const f32* p = data + ((x + (y * size.x)) * 4);
			painter.setBrush(QBrush(QColor(p[0] * 255, p[1] * 255, p[2] * 255)));
			painter.drawRect(QRect(box.origin.x + x * zoom, box.origin.y + y * zoom,  zoom, zoom));
		}
	}
}

void canvas::mouseReleaseEvent(QMouseEvent*) {
	cursor_release(hnd);
}

void canvas::mousePressEvent(QMouseEvent* e) {
	vec2D<i32> new_pos = to_imgspace(e);
	cursor_press(hnd, new_pos.x, new_pos.y);
	repaint();
}

void canvas::mouseMoveEvent(QMouseEvent* e) {
	vec2D<i32> new_pos = to_imgspace(e);
	cursor_drag(hnd, new_pos.x, new_pos.y);
	repaint();
}

rect<u16> canvas::viewport() {
	vec2D<u16> size;
	image_size(hnd, &size.x, &size.y);
	vec2D<u16> boxsize(size.x * zoom, size.y * zoom);
	vec2D<u16> origin((width() - boxsize.x) / 2,  (height() - boxsize.y) / 2);
	return ::rect<u16>(origin, boxsize);
}

vec2D<i32> canvas::to_imgspace(QMouseEvent* e) {
	::rect<u16> canvas_box = viewport();
	vec2D<f64> pos{e->position().x(), e->position().y()};
	vec2D<i32> p(pos.x - canvas_box.origin.x, pos.y - canvas_box.origin.y);
	return vec2D<i32>(p.x / zoom, p.y / zoom);
}
