#ifndef ES_REVENUE_MASTER_REPORT_H
#define ES_REVENUE_MASTER_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_revenuemasterreport.h"
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"

class ESRevenueMasterReport : public QWidget
{
	Q_OBJECT

public:
	ESRevenueMasterReport(QWidget *parent = 0);
	~ESRevenueMasterReport();

	Ui::RevenueMasterReport& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReport();
private:
	Ui::RevenueMasterReport ui;

	void printRow(KDReports::TableElement* tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report* report;
	QSignalMapper* m_generateReportSignalMapper;
};

#endif