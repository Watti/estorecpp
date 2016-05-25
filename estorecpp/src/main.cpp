#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include "easylogging++.h"
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../includes/utility/session.h"

INITIALIZE_EASYLOGGINGPP
using namespace std;
int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("./");
	QApplication a(argc, argv);
	//
	ifstream infile("resources//settings.res");

	while (infile)
	{
		string line;
		if (!getline(infile, line)) break;
		std::string name;
		std::istringstream is(line);
		std::string val;
		std::getline(is, val, '-');
		if (val.compare("BRANCH") == 0)
		{
			string branch;
			std::getline(is, branch, '-');
			branch = branch.substr(1, branch.size()-2);
			ES::Session::getInstance()->setBranchName(QString::fromUtf8(branch.data(), branch.size()));
		}
		else if (val.compare("DUMP_FOLDER") == 0)
		{
			string dumpPath;
			std::getline(is, dumpPath, '-');
			dumpPath = dumpPath.substr(1, dumpPath.size() - 2);
			ES::Session::getInstance()->setDumpPath(QString::fromUtf8(dumpPath.data(), dumpPath.size()));
		}
		
	}
	if (!infile.eof())
	{
		LOG(ERROR) << "Settings file cannot be read. path = " << "resources//settings.res";
	}
	//
	ESMainWindow w;
	w.showMaximized();
	LOG(INFO) << "Initializing the system...";
	ES::MainWindowHolder::instance()->setMainWindow(&w);

	// Create a report
	KDReports::Report report;

	// Add a text element for the title
	KDReports::TextElement titleElement(QObject::tr("Hello World!"));
	titleElement.setPointSize(18);
	report.addElement(titleElement, Qt::AlignHCenter);

	// add 20 mm of vertical space:
	report.addVerticalSpacing(20);

	// add some more text
	KDReports::TextElement textElement(QObject::tr("This is a report generated with KDReports"));
	report.addElement(textElement, Qt::AlignLeft);

	QPrinter printer;
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName("../print.pdf");

	report.print(&printer, 0);

	return a.exec();
}
