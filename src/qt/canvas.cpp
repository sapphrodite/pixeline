#include <QPainter>
#include <QMouseEvent>
#include <common/image_types.h>
#include "canvas.h"

canvas::canvas() {
	_data = image(std::vector<u8>(100 * 100 * 4, 255), vec2D<u16>(100, 100));
	_zoom = 5;
}

void canvas::paintEvent(QPaintEvent*) {
	::rect<u16> box = get_canvas_box();
	QPainter painter(this);

	// draw outline for reference
	painter.setPen(QPen());
	painter.drawRect(QRect(box.origin.x - 1, box.origin.y - 1 , box.size.x + 1, box.size.y + 1));
	painter.setPen(Qt::NoPen);

	for (int y = 0; y < _data.size().y; y++) {
		for (int x = 0; x < _data.size().x; x++) {
			rgba c = _data.get(x + (y * _data.size().x));
			painter.setBrush(QBrush(QColor(c.r, c.g, c.b)));
			painter.drawRect(QRect(box.origin.x + x * _zoom, box.origin.y + y * _zoom,  _zoom, _zoom));
		}
	}
}

void canvas::mousePressEvent(QMouseEvent* e) {
	::rect<i32> bounds(vec2D<i32>(0, 0), _data.size().to<i32>());
	diff d = active_tool.on_click(imagespace_coords(e), bounds);
	apply_diff(d);
}

void canvas::mouseMoveEvent(QMouseEvent* e) {
	::rect<i32> bounds(vec2D<i32>(0, 0), _data.size().to<i32>());
	diff d = active_tool.on_drag(imagespace_coords(e), bounds);
	apply_diff(d);
}


rect<u16> canvas::get_canvas_box() {
	vec2D<u16> size(_data.size().x * _zoom, _data.size().y * _zoom);
	vec2D<u16> origin((width() - size.x) / 2,  (height() - size.y) / 2);
	return ::rect<u16>(origin, size);
}

vec2D<i32> canvas::imagespace_coords(QMouseEvent* e) {
	::rect<u16> canvas_box = get_canvas_box();
	vec2D<i32> p(e->x() - canvas_box.origin.x, e->y() - canvas_box.origin.y);
	return vec2D<i32>(p.x / _zoom, p.y / _zoom);
}

void canvas::apply_diff(diff& d) {
	for (auto [pixel, color] : d) {
		if (pixel.x < _data.size().x && pixel.y < _data.size().y) {
			size_t index = pixel.x + (_data.size().x * pixel.y);
			_data.write(index, rgba{0, 0, 0, 0});
		}
	}

	//if (d.size() > 0) {
		repaint();
	//}
}

