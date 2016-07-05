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

#define SECOND_DISPLAY 0

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
		if (val.compare("SERVER") == 0)
		{
			std::string server;
			std::getline(is, server, '-');
			server = server.substr(1, server.size() - 2);
			ES::Session::getInstance()->setServerIP(QString::fromUtf8(server.data(), server.size()));
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

#if SECOND_DISPLAY
	ES::MainWindowHolder::instance()->openSecondaryDisplay();
	ES::Session::getInstance()->setBillItemFontSize(12);
	ES::Session::getInstance()->setBillTitle("HIRUNA MARKETING(PVT) LTD");
	ES::Session::getInstance()->setBillAddress("No.374, High level Road, Meegoda");
	ES::Session::getInstance()->setBillPhone("Phone : 011-2750485");
	ES::MainWindowHolder::instance()->openSecondaryDisplay();

#else
ES::Session::getInstance()->setBillItemFontSize(12);
ES::Session::getInstance()->setBillTitle("PUJITHA ENTERPRISES (PVT) LTD");
ES::Session::getInstance()->setBillAddress("No. 154, Kurugala, Padukka");
ES::Session::getInstance()->setBillPhone("Phone :  077-4784430 / 077-4784437");
ES::Session::getInstance()->setBillEmail("email : rapprasanna4@gmail.com");
#endif
//  	BackupThread b;
//  	b.start();

	return a.exec();
}
