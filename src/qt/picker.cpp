#include <QPainter>
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QCheckBox>

#include "picker.h"


QVBoxLayout* picker::_add_hsv_sliders() {
	auto* vbox = new QVBoxLayout;
	const char* hsv_labels[3] = {"H:", "S:", "V:"};

	for (int i = 0; i < 3; i++) {
		sliders[i] = new colormap;
		sliders[i]->set_axis(hsv::axis(i));
		sliders[i]->setMinimumSize(100, 20);
		hsv_boxes[i] = new QDoubleSpinBox;
		hsv_boxes[i]->setMaximum(hsv::max()[hsv::axis(i)]);

		connect(sliders[i], &colormap::value_changed, [&](hsv d) { update_color(d); });
		connect(hsv_boxes[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=] (double d){
			hsv_color[hsv::axis(i)] = d;
			update_color(hsv_color);
		});

		auto* hbox = new QHBoxLayout();
		hbox->addWidget(new QLabel(hsv_labels[i]));
		hbox->addWidget(sliders[i]);
		hbox->addWidget(hsv_boxes[i]);
		vbox->addLayout(hbox);
	}
	return vbox;
}

picker::picker(QWidget* parent, rgba rgb_in) : QDialog(parent) {
	hsv_color = to_hsv(rgb_in);
	const char* rgb_labels[3] = {"R:", "G:", "B:"};
	auto* rgb_vbox = new QVBoxLayout();
	for (int i = 0; i < 3; i++) {

		rgb_boxes[i] = new QDoubleSpinBox(this);
		rgb_boxes[i]->setMaximum(255);
		connect(rgb_boxes[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](int d){
			this->rgb_color[i] = d / 255.0f;
			update_color(rgb_color);
		});

		auto* hbox = new QHBoxLayout();
		hbox->addWidget(new QLabel(rgb_labels[i]));
		hbox->addWidget(rgb_boxes[i]);
		rgb_vbox->addLayout(hbox);
	}

	grid = new grid_parent;
	connect(grid->impl, &color_grid::value_changed, [=](hsv d) { update_color(d); });

	auto* br = new QHBoxLayout;
	br->addLayout(rgb_vbox);
	auto* accbutton = new QPushButton();
	br->addWidget(accbutton);
	connect(accbutton, &QPushButton::pressed, [=]() { emit value_changed(hsv_color); });

	auto* sliderbox =_add_hsv_sliders();
	sliderbox->addLayout(br);
	auto* hbox_outer = new QHBoxLayout(this);
	hbox_outer->addWidget(grid);
	hbox_outer->addLayout(sliderbox);
	update_color(hsv_color);
};

void picker::update_color(hsv c) { _update_color_impl(c, to_rgb(c)); }
void picker::update_color(rgba c) { _update_color_impl(to_hsv(c), c); }
void picker::_update_color_impl(hsv c, rgba c2) {
	if (!ignore_updates) {
		ignore_updates = true;
		for (int i = 0; i < 3; i++) {
			sliders[i]->update_color(c);
			hsv_boxes[i]->setValue(c[hsv::axis(i)]);
			rgb_boxes[i]->setValue(c2[i] * 255);
		}
		grid->impl->update_color(c);
		ignore_updates = false;
	}
}


void colormap::mouseMoveEvent(QMouseEvent * e) { mousePressEvent(e); };
void colormap::mousePressEvent(QMouseEvent * e) {
	f32 new_val = std::clamp((e->x() / f32(width())) * (hsv::max()[axis]), 0.0f, (hsv::max()[axis]));
	hsv_color[axis] = new_val;
	emit value_changed(hsv_color);
};

void colormap::set_axis(hsv::axis axis_in) { axis = axis_in; }
void colormap::update_color(hsv new_color) {
	hsv_color = new_color;
	repaint();
}

void colormap::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	painter.setPen(Qt::NoPen);

	hsv itr_color = hsv_color;
	itr_color[axis] = 0;
	f32 div_size = (hsv::max()[axis]) / width();
	int num_divs = div_size > 0 ? 1 : floor(1.0 / div_size);
	div_size = div_size * num_divs;

	for (int x = 0; x < width(); x += num_divs) {
		rgba c = to_rgb(itr_color);
		painter.setBrush(QBrush(QColor(c.r, c.g, c.b)));
		painter.drawRect(QRect(x, 0, num_divs, height()));
		itr_color[axis] += div_size;
	}

	int cursor_location = int(hsv_color[axis] / (hsv::max()[axis])) * width();
	painter.setBrush(QBrush(QColor(255, 255, 255)));
	painter.drawRect(QRect(cursor_location, 0, 1, height()));
}


color_grid::color_grid(hsv::axis x_in, hsv::axis y_in) : x_axis(x_in), y_axis(y_in) { setMinimumSize(100, 20); };
void color_grid::update_color(hsv new_color) {
	hsv_color = new_color;
	repaint();
}

void color_grid::mouseDoubleClickEvent(QMouseEvent * e) {
	vec2D<u16> pps(width() / grid_size, height() / grid_size);
	vec2D<i16> square((e->x() / pps.x) - (grid_size / 2), (e->y() / pps.y) - (grid_size / 2));

	hsv itr_color = hsv_color;
	itr_color[x_axis] += steps.x * square.x;
	itr_color[y_axis] += steps.y * square.y;

	hsv_color = itr_color;
	emit value_changed(hsv_color);
};

void color_grid::paintEvent(QPaintEvent*) {
	auto* p = reinterpret_cast<grid_parent*>(parent());
	QPainter painter(this);
	if (!p->gridlines_toggle->isChecked()) {
		painter.setPen(Qt::NoPen);
	}

	steps = vec2D<f32>(p->xstep_box->value(), p->ystep_box->value());
	hsv itr_color = hsv_color;
	itr_color[x_axis] = std::clamp(itr_color[x_axis] - steps.x * (grid_size / 2), 0.0f, (hsv::max()[x_axis]));
	itr_color[y_axis] = std::clamp(itr_color[y_axis] - steps.y * (grid_size / 2), 0.0f, (hsv::max()[y_axis]));
	f32 x_base = itr_color[x_axis];

	vec2D<u16> div_size(width() / grid_size, height() / grid_size);
	for (int y = 0; y < grid_size; y++) {
		for (int x = 0; x < grid_size; x++) {
			itr_color[x_axis] = std::min(itr_color[x_axis] + steps.x, (hsv::max()[x_axis]));
			rgba c = to_rgb(itr_color);
			painter.setBrush(QBrush(QColor(c.r * 255, c.g * 255, c.b * 255)));
			painter.drawRect(QRect(x * div_size.x, y * div_size.y, div_size.x, div_size.y));
		}
		itr_color[x_axis] = x_base;
		itr_color[y_axis] = std::min(itr_color[y_axis] + steps.y, (hsv::max()[y_axis]));
	}
}


grid_parent::grid_parent() {
	impl = new color_grid(hsv::axis::h, hsv::axis::s);
	impl->setMinimumSize(11 * 15, 11 * 15);

	QVBoxLayout* grid_vbox = new QVBoxLayout(this);
	QHBoxLayout* temp = new QHBoxLayout;
	temp->addWidget(new QLabel("Size:"));

	auto add_pushbutton = [&](int incr) {
		QPushButton* zoom = new QPushButton;
		connect(zoom, &QPushButton::pressed, [=](){
			impl->grid_size += incr;
			impl->repaint();
		});
		temp->addWidget(zoom);
	};
	add_pushbutton(2);
	add_pushbutton(-2);

	grid_vbox->addWidget(impl);
	grid_vbox->addLayout(temp);


	temp->addWidget(new QLabel("Gridlines:"));
	gridlines_toggle = new QCheckBox;
	connect(gridlines_toggle, &QCheckBox::stateChanged, [=](int){ impl->repaint(); });
	temp->addWidget(gridlines_toggle);

	temp = new QHBoxLayout;
	auto init_step_box = [&](QDoubleSpinBox*& box, const char* str) {
		box = new QDoubleSpinBox();
		temp->addWidget(new QLabel(str));
		temp->addWidget(box);
		connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double) { impl->repaint(); });
	};
	init_step_box(xstep_box, "X Step:");
	init_step_box(ystep_box, "Y Step:");

	grid_vbox->addLayout(temp);
}
