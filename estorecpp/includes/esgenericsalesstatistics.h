#ifndef SALES_REPORT_H
#define SALES_REPORT_H

#include "view/gobchartsview.h"
#include "ui_genericsalesreportwidget.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

class ESGenericSalesStatistics : public QWidget
{
	Q_OBJECT

public:
	ESGenericSalesStatistics(QWidget *parent = 0);

	~ESGenericSalesStatistics();

	Ui::GenericSalesReport& getUI() { return ui; };

	GobChartsView* generatemonthlySalesChart();
	GobChartsView* generateAnnualSalesChart();

	void generateReport();

private:

	Ui::GenericSalesReport ui;
	
};

#endif