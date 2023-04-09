#ifndef PALETTE_H
#define PALETTE_H

#include <QWidget>
#include <common/image_types.h>

class palette : public QWidget {
	Q_OBJECT
public:
	rgba& get(size_t index);
	void flip_left();
	void flip_right();
	void zoom_in();
	void zoom_out();

	void mouseDoubleClickEvent( QMouseEvent * e ) override;
	void paintEvent(QPaintEvent *event) override;
private:
	rgba _colors[256];
	u8 _page_index = 0;
	u8 _zoom_level = 1;
};

#endif // PALETTE_H
