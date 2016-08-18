#ifndef ESSTOCK_REPORT_H
#define ESSTOCK_REPORT_H

#include "ui_stockreport.h"
#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_stockreportcontainer.h"

class ESStockReport : public QWidget
{
	Q_OBJECT

public:
	ESStockReport(QWidget *parent = 0);

	void displayResults();

	~ESStockReport();

	Ui::StockReportWidget& getUI() { return ui; };

	public slots:
	void slotGenerate();
	void slotPrint(QPrinter* printer);
	
private:
	Ui::StockReportWidget ui;
	KDReports::Report* m_report;

};

#endif