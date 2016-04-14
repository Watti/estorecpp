#ifndef SALES_REPORT_H
#define SALES_REPORT_H

#include "view/gobchartsview.h"
#include "ui_genericsalesreportwidget.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include "widget/gobchartswidget.h"
#include "QStandardItemModel"

class ESGenericSalesStatistics : public QWidget
{
	Q_OBJECT

public:
	ESGenericSalesStatistics(QWidget *parent = 0);

	~ESGenericSalesStatistics();

	Ui::GenericSalesReport& getUI() { return ui; };


	

private:

	Ui::GenericSalesReport ui;
	QWidget* generateChart(QStandardItemModel* model, GobChartsType chartType, const QString& title);
	QWidget* generateMonthlySalesChart();
	QWidget* generateAnnualSalesChart();
	QWidget * generatePayementTypeSalesChart();

};

#endif