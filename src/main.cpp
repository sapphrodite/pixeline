#include "core/api.h"
#include "qt/canvas.h"
#include <QApplication>
#include <QShortcut>

int main(int argc, char *argv[]) {
	handle* hnd = handle_new();
	image_new(hnd, 100, 100);
	pal_set(hnd, 0, rgba{0, 0, 0, 1});

	QApplication a(argc, argv);
	canvas c(hnd);
	c.setMinimumSize(50, 50);
	c.show();

	auto* undohook = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), &c);
	QObject::connect(undohook, &QShortcut::activated, [&]() {
		undo(hnd);
		c.repaint();
	});

	auto* redohook = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), &c);
	QObject::connect(redohook, &QShortcut::activated, [&]() {
		redo(hnd);
		c.repaint();
	});

	auto* pencilhook = new QShortcut(QKeySequence(Qt::Key_L), &c);
	QObject::connect(pencilhook, &QShortcut::activated, [&]() {
		tool_select(hnd, tool::pencil);
	});

	auto* fillhook = new QShortcut(QKeySequence(Qt::Key_B), &c);
	QObject::connect(fillhook, &QShortcut::activated, [&]() {
		tool_select(hnd, tool::fill);
	});

	return a.exec();
}
