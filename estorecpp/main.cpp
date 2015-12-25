#include "esmainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ESMainWindow w;
	w.showMaximized();
	return a.exec();
}
