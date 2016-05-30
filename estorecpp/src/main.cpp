#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include "utility/session.h"
#include "easylogging++.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
		else if (val.compare("DUMP_FOLDER") == 0)
		{
			std::string dumpPath;
			std::getline(is, dumpPath, '-');
			dumpPath = dumpPath.substr(1, dumpPath.size() - 2);
			ES::Session::getInstance()->setDumpPath(QString::fromUtf8(dumpPath.data(), dumpPath.size()));
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

	ESMainWindow w;
	w.showMaximized();
	LOG(INFO) << "Initializing the system...";
	ES::MainWindowHolder::instance()->setMainWindow(&w);

	return a.exec();
}
