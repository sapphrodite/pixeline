#include <QApplication>
#include <QBoxLayout>
#include <QPushButton>
#include <QSplitter>

#include "palette/palette.h"
#include "canvas/canvas.h"
#include "palette/picker.h"


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
    QApplication a(argc, argv);
    auto* main_panes = new QSplitter(Qt::Orientation::Horizontal);
    palette p;
    canvas c;

    p.get(0) = color(255, 9, 255, 255);
    p.get(1) = color(2, 9, 50, 255);
    p.get(2) = color(25, 9, 255, 255);
    p.get(3) = color(255, 9, 5, 255);
    p.get(4) = color(255, 100, 255, 255);
    p.get(5) = color(55, 100, 255, 255);
    p.get(6) = color(255, 150, 25, 255);

    main_panes->addWidget(leftpane(p));
    main_panes->addWidget(&c);
    main_panes->show();
    return a.exec();
}
