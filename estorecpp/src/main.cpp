#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("./");
	QApplication a(argc, argv);

	ESMainWindow w;
	w.showMaximized();
	LOG(INFO) << "Initializing the system...";
	ES::MainWindowHolder::instance()->setMainWindow(&w);

	return a.exec();
}
