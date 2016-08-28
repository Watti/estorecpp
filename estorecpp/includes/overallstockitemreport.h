#ifndef ES_OVERALL_STOCK_REPORT_H
#define ES_OVERALL_STOCK_REPORT_H

#include "ui_overallstockitemreport.h"
#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_stockreportcontainer.h"

class ESOverallStockItemReport : public QWidget
{
	Q_OBJECT

public:
	ESOverallStockItemReport(QWidget *parent = 0);

	void displayResults();

	~ESOverallStockItemReport();

	Ui::OverallStockItemsReportWidget& getUI() { return ui; };

	public slots:
	void slotGenerate();
	void slotPrint(QPrinter* printer);
	void slotPrev();
	void slotNext();

private:
	Ui::OverallStockItemsReportWidget ui;
	KDReports::Report* m_report;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;

};

#endif