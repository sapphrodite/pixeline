#include "canvas.h"
#include <core/api.h>
#include <QMouseEvent>
#include <QPainter>

canvas::canvas(handle* hnd) : hnd(hnd) {
	zoom = 5;
}

void canvas::paintEvent(QPaintEvent*) {
	::rect box = viewport();
	QPainter painter(this);

	// draw outline for reference
	painter.setPen(QPen());
	painter.drawRect(QRect(box.origin.x - 1, box.origin.y - 1 , box.size.x + 1, box.size.y + 1));
	painter.setPen(Qt::NoPen);

	vec2u size = image_size(hnd);
	const f32* data = image_data(hnd);
	for (unsigned y = 0; y < size.y; y++) {
		for (unsigned x = 0; x < size.x; x++) {
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
	cursor_press(hnd, to_imgspace(e));
	repaint();
}

void canvas::mouseMoveEvent(QMouseEvent* e) {
	cursor_drag(hnd, to_imgspace(e));
	repaint();
}

rect canvas::viewport() {
	vec2u boxsize = image_size(hnd) * vec2u{zoom, zoom};
	vec2i origin((width() - boxsize.x) / 2,  (height() - boxsize.y) / 2);
	return ::rect(origin, boxsize.to<int>());
}

vec2i canvas::to_imgspace(QMouseEvent* e) {
	::rect canvas_box = viewport();
	vec2<f64> pos{e->position().x(), e->position().y()};
	vec2i p(pos.x - canvas_box.origin.x, pos.y - canvas_box.origin.y);
	return vec2i(p.x / zoom, p.y / zoom);
}
