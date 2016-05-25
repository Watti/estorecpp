#ifndef ESSTOCK_REPORT_H
#define ESSTOCK_REPORT_H

#include "ui_stockreport.h"
#include <QtGui>

class ESStockReport : public QWidget
{
	Q_OBJECT

public:
	ESStockReport(QWidget *parent = 0);
	~ESStockReport();

	Ui::StockReportWidget& getUI() { return ui; };

	public slots:
	void slotGenerate();
	
private:
	Ui::StockReportWidget ui;

};

#endif