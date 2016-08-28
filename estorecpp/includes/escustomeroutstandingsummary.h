#ifndef ES_CUSTOMER_OUTSTANDING_SUMMARY_H
#define ES_CUSTOMER_OUTSTANDING_SUMMARY_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"
#include "ui_customeroutstandingsummary.h"

class ESCustomerOutstandingSummary : public QWidget
{
	Q_OBJECT

public:
	ESCustomerOutstandingSummary(QWidget *parent = 0);
	~ESCustomerOutstandingSummary();

	Ui::CustomerOutstandingSummary& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReport();
private:
	Ui::CustomerOutstandingSummary ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report* report;
	QSignalMapper* m_generateReportSignalMapper;
};

#endif