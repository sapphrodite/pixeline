#include <QApplication>
#include <QBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QShortcut>

#include "qt/palette.h"
#include "qt/canvas.h"
#include "qt/picker.h"

#include "core/selection.h"
#include "core/api.h"
#include <cstdio>
#include <assert.h>

// ensure empty selections don't allow iteration over bogus elements
bool test_empty(selection t) {
	for (auto marked : t)
		return false;
	return true;
}

// test if mark, exists, and clear operations works
bool test_mark_clear_exists(selection t) {
	t.mark({1, 3});
	bool mark_passed = t.exists({1, 3});
	t.clear({1, 3});
	return !t.exists({1, 3}) && mark_passed;
}

selection fill_sel_forwards(vec2D<u16> bounds) {
	selection s;
	for (int y = 0; y < bounds.y; y++)
		for (int x = 0; x < bounds.x; x++)
			s.mark({x, y});
	return s;
}

bool fill_sel_backwards(vec2D<u16> bounds) {
	printf("Testing backwards selection fills...\n");
	selection s;
	for (int y = bounds.y - 1; y >= 0; y--)
		for (int x = bounds.x - 1; x >= 0; x--)
			s.mark({x, y});

	size_t num_elems = 0;
	for (auto marked : s) {
		// printf("pos is %i, %i\n", marked.x, marked.y);
		num_elems++;
	}
	printf("read %li elements, should be %li\n", num_elems, size_t{bounds.x * bounds.y});
	return num_elems == size_t{bounds.x * bounds.y};
}

bool perf_test() {
	vec2D<u16> bounds(256, 256);
	auto t = fill_sel_forwards(bounds);

	size_t num_elems = 0;
	for (auto marked : t) {
		//printf("pos is %i, %i\n", marked.x, marked.y);
		num_elems++;
	}
	printf("num elems is %li, should be %li\n", num_elems, size_t{bounds.x * bounds.y});
	return num_elems == size_t{bounds.x * bounds.y};
}


bool color_test() {
	vec2D<u16> bounds(16, 16);
	diff t;

	for (int y = 0; y < bounds.y; y++) {
		for (int x = 0; x < bounds.x; x++) {
			rgba color;
			if (x == y) 
				color = rgba{1, 1, 1, 1};
			else 
				color = rgba{0, 0, 0, 0};
			t.insert({x, y}, color);
		}
	}
	size_t num_elems = 0;
	for (auto [pos, color] : t) {
		//printf("%3i, %3i has color (%3.1f)\n", pos.x, pos.y, color.r);
		num_elems++;
	}
	return num_elems == bounds.x * bounds.y;

}


int tests() {

	selection t;
	// t.mark({16000, 16000});
	t.mark({64, 64});
	t.mark({2, 3});
	t.mark({1, 3});
	t.mark({12, 3});
	assert(t.exists({1, 3}));
	t.clear({2, 3});
	for (auto marked : t) {
		printf("pos is %i, %i\n", marked.x, marked.y);
	}

	std::vector<selection> test_trees;
	test_trees.emplace_back(selection());
	test_trees.emplace_back(selection());

	bool all_passed = perf_test() && color_test() && fill_sel_backwards({256, 256});

	for (auto& tree : test_trees) {
		all_passed = all_passed && test_empty(tree);
		all_passed = all_passed && test_mark_clear_exists(tree);
	}

	printf("All tests %s\n", all_passed ? "passed" : "failed");
	return 0;
}

QWidget* leftpane(palette& p) {
	auto* passthrough = new QWidget;
	auto* vbox = new QVBoxLayout(passthrough);

	auto* hbox = new QHBoxLayout;
	auto* button1 = new QPushButton();
	hbox->addWidget(button1);
	auto* button2 = new QPushButton();
	hbox->addWidget(button2);
	auto* button3 = new QPushButton();
	hbox->addWidget(button3);
	auto* button4 = new QPushButton();
	hbox->addWidget(button4);

	vbox->addWidget(&p);
	vbox->addLayout(hbox);
	return passthrough;
}

int main(int argc, char *argv[]) {
	handle* hnd = handle_alloc();
	new_image(hnd, 100, 100);

	tests();
	QApplication a(argc, argv);
	auto* main_panes = new QSplitter(Qt::Orientation::Horizontal);
	palette p;
	canvas c(hnd);
	
	QObject::connect(&p, &palette::color_select, &c, &canvas::select_color);	

	p.get(0) = rgba(1, 0.1, 0.5, 1);
	p.get(1) = rgba(0, 0, 0, 0);
	p.get(2) = rgba(25, 9, 255, 255);
	p.get(3) = rgba(255, 9, 5, 255);
	p.get(4) = rgba(255, 100, 255, 255);
	p.get(5) = rgba(55, 100, 255, 255);
	p.get(6) = rgba(255, 150, 25, 255);

	for (int i= 0; i < 7; i++) {
		set_pal_color(hnd, i, p.get(i));
	}	
	main_panes->addWidget(leftpane(p));
	main_panes->addWidget(&c);
	main_panes->show();

	auto* undohook = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), main_panes);
	auto* redohook = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), main_panes);
	QObject::connect(undohook, &QShortcut::activated, [&]() {
		undo(hnd);
		c.repaint();
	});
	QObject::connect(redohook, &QShortcut::activated, [&]() {
		redo(hnd);
		c.repaint();
	});
	return a.exec();
}
