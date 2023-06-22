#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <core/image_types.h>
#include <core/api.h>
#include "tool.h"

class canvas : public QWidget {
	Q_OBJECT
public:
	canvas(handle* hnd);

	void paintEvent(QPaintEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;

public slots:
	void select_color(u8);
private:
	handle* hnd;
	u8 _zoom;
	vec2D<i32> last_pos;
	bool tool_active = false;
	u8 active_color = 0;

	vec2D<u16> size();
	::rect<u16> get_canvas_box();
	vec2D<i32> imagespace_coords(QMouseEvent*);
	void apply_diff(diff& d);
};

#endif // CANVAS_H

