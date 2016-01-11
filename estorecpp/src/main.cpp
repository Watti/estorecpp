#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include <QtWidgets/QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	ESMainWindow w;
	w.showMaximized();

	ES::MainWindowHolder::instance()->setMainWindow(&w);

	return a.exec();
}
