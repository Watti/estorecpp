#include "esmainwindow.h"
#include "utility/esmainwindowholder.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include "easylogging++.h"
#include <KDReportsReport.h>
#include <KDReportsTextElement.h>

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath("./");
	QApplication a(argc, argv);

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
