#ifndef ES_CUSTOMER_WISE_SALES_SUMMARY_H
#define ES_CUSTOMER_WISE_SALES_SUMMARY_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"
#include "ui_customerwisesalessummary.h"

class ESCustomerWiseSalesSummary : public QWidget
{
	Q_OBJECT

public:
	ESCustomerWiseSalesSummary(QWidget *parent = 0);
	~ESCustomerWiseSalesSummary();

	Ui::CustomerWiseSalesSummary& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReport();
private:
	Ui::CustomerWiseSalesSummary ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report* report;
	QSignalMapper* m_generateReportSignalMapper;
};

#endif