#ifndef PICKER_H
#define PICKER_H

#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QBoxLayout>
#include <QCheckBox>

#include <core/image_types.h>
#include <common/coordinate_types.h>

class colormap;
class grid_parent;

class picker : public QDialog {
	Q_OBJECT
public:
	picker(QWidget* parent, rgba rgb_in);
private:
	colormap* sliders[3];
	QDoubleSpinBox* hsv_boxes[3];
	QDoubleSpinBox* rgb_boxes[3];
	QLineEdit hex_box;

	grid_parent* grid;

	hsv hsv_color;
	rgba rgb_color;

	bool ignore_updates;
	void update_color(hsv);
	void update_color(rgba);
	void _update_color_impl(hsv, rgba);
	QVBoxLayout* _add_hsv_sliders();
signals:
	void value_changed(hsv);
};


class colormap : public QWidget {
	Q_OBJECT
public:
	void set_axis(int in);
	void update_color(hsv new_color);

	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent * e) override;
	void mouseMoveEvent(QMouseEvent * e) override;
signals:
	void value_changed(hsv);
private:
	int axis;
	hsv hsv_color;
};


class color_grid : public QWidget {
	Q_OBJECT
public:
	color_grid() = default;
	color_grid(int x_in, int y_in);
	void update_color(hsv new_color);
	void paintEvent(QPaintEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;
//private:
	int x_axis;
	int y_axis;
	hsv hsv_color;

	vec2<f32> steps = vec2<f32>(1, 1);
	u8 grid_size = 7;
signals:
	void value_changed(hsv);
};


class grid_parent : public QWidget {
public:
	grid_parent();
	color_grid* impl;
	QCheckBox* gridlines_toggle;
	QDoubleSpinBox* xstep_box;
	QDoubleSpinBox* ystep_box;
};

#endif // PICKER_H
