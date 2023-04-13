#ifndef PALETTE_H
#define PALETTE_H

#include <QWidget>
#include <core/api.h>

class palette : public QWidget {
	Q_OBJECT
public:
	palette(handle* hnd) : hnd(hnd) {};

	rgba get(size_t index);
	void flip_left();
	void flip_right();
	void zoom_in();
	void zoom_out();

	void paintEvent(QPaintEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
private:
	handle* hnd;
	u8 _page_index = 0;
	u8 _zoom_level = 1;
signals:
	void color_select(u8);
};

#endif // PALETTE_H
