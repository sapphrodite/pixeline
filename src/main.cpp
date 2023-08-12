#include "core/api.h"
#include "qt/canvas.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	handle* hnd = handle_new();
	image_new(hnd, 100, 100);
	pal_set(hnd, 0, rgba{0, 0, 0, 1});

	QApplication a(argc, argv);
	canvas c(hnd);
	c.show();
	return a.exec();
}
