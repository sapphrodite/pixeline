#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <common/image_types.h>
#include "tool.h"

class canvas : public QWidget {
	Q_OBJECT
public:
	canvas();

	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
private:
	image _data;
	u8 _zoom;
	pencil active_tool;

	::rect<u16> get_canvas_box();
	vec2D<i32> imagespace_coords(QMouseEvent*);
	void apply_diff(diff& d);
};

#endif // CANVAS_H

