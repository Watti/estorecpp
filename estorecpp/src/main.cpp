#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include "utility/session.h"
#include "easylogging++.h"
#include <QtWidgets/QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "utility/session.h"
#include "esbackuprestore.h"
#include "essecondarydisplay.h"

INITIALIZE_EASYLOGGINGPP

void readSettings()
{
	std::ifstream infile("resources\\settings.res");

	while (infile)
	{
		std::string line;
		if (!std::getline(infile, line)) break;
		std::string name;
		std::istringstream is(line);
		std::string val;
		std::getline(is, val, '-');
		if (val.compare("BRANCH") == 0)
		{
			std::string branch;
			std::getline(is, branch, '-');
			branch = branch.substr(1, branch.size() - 2);
			ES::Session::getInstance()->setBranchName(QString::fromUtf8(branch.data(), branch.size()));
		}
		else if (val.compare("BACKUP_FOLDER") == 0)
		{
			std::string dumpPath;
			std::getline(is, dumpPath, '-');
			dumpPath = dumpPath.substr(1, dumpPath.size() - 2);
			ES::Session::getInstance()->setBackupPath(QString::fromUtf8(dumpPath.data(), dumpPath.size()));
		}
		else if (val.compare("PRICE_CHANGE_ACTION") == 0)
		{
			std::string dumpPath;
			std::getline(is, dumpPath, '-');
			dumpPath = dumpPath.substr(1, dumpPath.size() - 2);
			QString action = QString::fromUtf8(dumpPath.data(), dumpPath.size());
			if (action == "block")
			{
				ES::Session::getInstance()->setLowerPriceBlocked(true);
			}
			else
			{
				ES::Session::getInstance()->setLowerPriceBlocked(false);
			}
		}

	}
	if (!infile.eof())
	{
		LOG(ERROR) << "Settings file cannot be read. path = " << "resources\\settings.res";
	}
}

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("./");
	QApplication a(argc, argv);

	readSettings();

	ESMainWindow mainWindow;

	QRect rect1 = QApplication::desktop()->screenGeometry(0);
	mainWindow.move(QPoint(rect1.x(), rect1.y()));
	mainWindow.resize(rect1.width(), rect1.height());
	mainWindow.showMaximized();

	LOG(INFO) << "Initializing the system...";

	ES::MainWindowHolder::instance()->setMainWindow(&mainWindow);
	ES::MainWindowHolder::instance()->openSecondaryDisplay();

 	BackupThread b;
 	b.start();

	return a.exec();
}
