#include "esgenericsalesstatistics.h"
#include "view/gobchartsfactory.h"
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QColor>
#include <QGridLayout>
#include "QPdfWriter"
#include "QPainter"
#include "QPagedPaintDevice"
#include "QTextDocument"
#include "QPageLayout"
#include "QAbstractTextDocumentLayout"
#include "QPrinter"
#include <widget/gobchartswidget.h>
#include "QLabel"
#include "QBoxLayout"

ESGenericSalesStatistics::ESGenericSalesStatistics(QWidget *parent /* = 0 */)
:QWidget(parent)
{
	ui.setupUi(this);

	ui.gridLayout->addWidget(generateMonthlySalesChart(), 0, 0);
	ui.gridLayout->addWidget(generateAnnualSalesChart(), 0,1);
	//ui.gridLayout->addWidget(monthlySalesReport(), 1, 0);
	//ui.gridLayout->addWidget(monthlySalesReport(), 1, 1);
	//generateReport();
}

ESGenericSalesStatistics::~ESGenericSalesStatistics()
{

}

GobChartsWidget* ESGenericSalesStatistics::generateMonthlySalesChart()
{
	GobChartsWidget* chartWidget = new GobChartsWidget;
	//chartWidget->setWindowTitle("Monthly Sales");
	//chartWidget->setObjectName("MontHly Sales");
	QItemSelectionModel* salesSelectionModel;
	chartWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QFont font;
	chartWidget->setFont(font);
	//All the bill amount vs date
	QStandardItemModel *model = new QStandardItemModel(this);
	QStringList list;
	list.append("Monthly Sales");
	model->setVerticalHeaderLabels(list);
	QSqlQuery query("SELECT SUM(amount) as total, YEAR(date) as y, MONTHNAME(date) as m FROM bill WHERE deleted = 0 AND status = 1 GROUP BY MONTH(date)");
	int row = 0, col = 0;
	while (query.next())
	{
		QString yearMonth = query.value("y").toString() + "(" + query.value("m").toString()+")";
		QStandardItem* monthItem = new QStandardItem(yearMonth);
		model->setItem(row, col++, monthItem);
		QString total =  query.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + yearMonth);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	chartWidget->setModel(model);
	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	chartWidget->setSelectionModel(selectionModel);
	chartWidget->createChart(BAR);

//  	QLabel i_label("Monthly Sales", chartWidget);
// 	i_label.setText("Monthly Sales Report");
//  	QVBoxLayout *vbl = new QVBoxLayout(chartWidget);
// 	vbl->addWidget(&i_label);
	chartWidget->show();	
	return chartWidget;
}

GobChartsWidget* ESGenericSalesStatistics::generateAnnualSalesChart()
{	
	GobChartsWidget* chartWidget = new GobChartsWidget;
	chartWidget->setWindowTitle("Annual Sales");
	QAbstractItemModel* salesChartModel;
	QItemSelectionModel* salesSelectionModel;
	chartWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QFont font;
	chartWidget->setFont(font);

	//All the bill amount vs date
	QStandardItemModel *model = new QStandardItemModel(this);
	QSqlQuery query("SELECT SUM(amount) as total, YEAR(date) as y FROM bill WHERE deleted = 0 AND status = 1 GROUP BY YEAR(date)");
	int row = 0, col = 0;
	while (query.next())
	{
		QString year = query.value("y").toString();
		QStandardItem* monthItem = new QStandardItem(year);
		model->setItem(row, col, monthItem);
		col++;
		
		QString total = query.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + year);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	chartWidget->setModel(model);
	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	chartWidget->setSelectionModel(selectionModel);

	chartWidget->createChart(PIE);
	chartWidget->show();

	return chartWidget;
}

void ESGenericSalesStatistics::generateReport()
{
	QTextDocument doc;
	QPdfWriter writer("AnualReport.pdf");
	QPainter painter(&writer);
	

	writer.setPageSize(QPagedPaintDevice::A4);
	painter.drawText(0, 0, "Here we go");
	//painter.drawPixmap(QRect(0, 0, writer.logicalDpiX()*8.3, writer.logicalDpiY()*11.7), QPixmap("penguin.jpg"));

	painter.end();
	doc.drawContents(&painter);
}
