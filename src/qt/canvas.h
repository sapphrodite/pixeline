#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <core/image_types.h>
#include <core/api.h>

class canvas : public QWidget {
	Q_OBJECT
public:
	canvas(handle* hnd);

	void paintEvent(QPaintEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
private:
	handle* hnd;
	u8 _zoom;
	vec2D<i32> last_pos;

	vec2D<u16> size();
	::rect<u16> get_canvas_box();
	vec2D<i32> imagespace_coords(QMouseEvent*);
};

#endif // CANVAS_H

