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

	ui.gridLayout->addWidget(generateMonthlySalesChart());
	ui.gridLayout->addWidget(generateAnnualSalesChart());
}

ESGenericSalesStatistics::~ESGenericSalesStatistics()
{

}

QWidget* ESGenericSalesStatistics::generateMonthlySalesChart()
{
	GobChartsWidget* chartWidget = new GobChartsWidget;
	chartWidget->setMinimumHeight(400);
	chartWidget->setMinimumWidth(500);
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
	QString title("Monthly Sales Summary - Recent Past");
	return generateChart(model, BAR, title);
}

QWidget* ESGenericSalesStatistics::generateAnnualSalesChart()
{	
	GobChartsWidget* chartWidget = new GobChartsWidget;
	chartWidget->setMinimumHeight(400);
	chartWidget->setMinimumWidth(500);
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
	QString title("Annual Sales Summary");
	return generateChart(model, PIE, title);
}

QWidget* ESGenericSalesStatistics::generateChart(QStandardItemModel* model, GobChartsType chartType, const QString& titleText)
{
	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
	QLabel* title = new QLabel(titleText);
	title->setAlignment(Qt::AlignCenter);
	GobChartsWidget* chartWidget = new GobChartsWidget(mainWidget);
	QAbstractItemModel* salesChartModel;
	QItemSelectionModel* salesSelectionModel;
	chartWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	QFont font;
	font.setPixelSize(14);
	font.setBold(true);

	chartWidget->setModel(model);
	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	chartWidget->setSelectionModel(selectionModel);

	chartWidget->createChart(chartType);
	chartWidget->show();

	title->setFont(font);
	mainWidget->setLayout(mainLayout);
	mainLayout->addWidget(title);
	mainLayout->addWidget(chartWidget);

	return mainWidget;
}