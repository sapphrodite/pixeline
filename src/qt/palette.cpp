#include <QPainter>
#include <QMouseEvent>

#include "palette.h"
#include "picker.h"
#include <common/assertion.h>


void palette::flip_left() { _page_index = std::max(_page_index, u8(_page_index - 1)); }
void palette::flip_right() { _page_index = std::max(_page_index + 1, 256 / (_page_index * 2 << _zoom_level)); }
void palette::zoom_in() { _zoom_level = std::clamp(_zoom_level + 1, 0, 3); }
void palette::zoom_out() { _zoom_level = std::clamp(_zoom_level - 1, 0, 3); }
rgba palette::get(size_t index) { return get_pal_color(hnd, index); }

void palette::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	painter.setPen(QPen());

	u8 size = (2 << _zoom_level);
	int square_size = ((std::min(width(), height()) - 1) / size) - 1;
	int total_size = (square_size + 1) * size + 1;

	painter.setBrush(QBrush(QColor(0, 255, 255)));
	painter.drawRect(QRect(0, 0,  total_size, total_size));

	size_t offset = _page_index * (2 << _zoom_level);
	for (int i = 0; i < size * size; i++) {
		rgba c = get(i + offset);
		painter.setBrush(QBrush(QColor(c.r, c.g, c.b)));
		int x_pos = (i % size) * (square_size + 1) + 1;
		int y_pos = (i / size) * (square_size + 1) + 1;
		painter.drawRect(QRect(x_pos, y_pos,  square_size, square_size));
	}
}

void palette::mouseDoubleClickEvent(QMouseEvent* e) {
	u8 tiles_per_side = 2 << _zoom_level;
	u8 pixels_per_square = std::min(height(), width()) / tiles_per_side;
	int x_index = (e->x() / pixels_per_square);
	int y_index = (e->y() / pixels_per_square);
	auto* p = new picker(this, get_pal_color(hnd, x_index + (y_index * tiles_per_side)));
	p->show();

	connect(p, &picker::value_changed, [=] (hsv d) {
		set_pal_color(hnd, x_index + (y_index * tiles_per_side), to_rgb(d));
		repaint();
	});
}

void palette::mousePressEvent(QMouseEvent* e) {
	u8 tiles_per_side = 2 << _zoom_level;
	u8 pixels_per_square = std::min(height(), width()) / tiles_per_side;
	int x_index = (e->x() / pixels_per_square);
	int y_index = (e->y() / pixels_per_square);
	emit color_select(x_index + (y_index * tiles_per_side));
}
