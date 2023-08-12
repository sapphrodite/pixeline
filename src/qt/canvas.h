#ifndef CANVAS_H
#define CANVAS_H

#include <core/api.h>
#include <include/coordinate_types.h>
#include <QWidget>

class canvas : public QWidget {
public:
	canvas(handle* hnd);

	void paintEvent(QPaintEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
private:
	handle* hnd;
	u8 zoom;

	::rect<u16> viewport();
	vec2D<i32> to_imgspace(QMouseEvent*);
};

#endif // CANVAS_H
