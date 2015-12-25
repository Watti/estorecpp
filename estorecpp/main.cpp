#include "esmainwindow.h"
#include <QtWidgets/QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	ESMainWindow w;
	w.showMaximized();

	return a.exec();
}
