#include "esgenericsalesstatistics.h"
#include "view/gobchartsfactory.h"
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QColor>
#include <QGridLayout>

ESGenericSalesStatistics::ESGenericSalesStatistics(QWidget *parent /* = 0 */)
:QWidget(parent)
{
	ui.setupUi(this);

	ui.gridLayout->addWidget(monthlySalesReport(), 0, 0);
	ui.gridLayout->addWidget(monthlySalesReport(), 0, 1);
	ui.gridLayout->addWidget(monthlySalesReport(), 1, 0);
	ui.gridLayout->addWidget(monthlySalesReport(), 1, 1);

}

ESGenericSalesStatistics::~ESGenericSalesStatistics()
{

}

GobChartsView* ESGenericSalesStatistics::monthlySalesReport()
{
	GobChartsView* gobSalesChartsView = GobChartsFactory::getInstance()->createChart(BAR, this);
	QAbstractItemModel* salesChartModel;
	QItemSelectionModel* salesSelectionModel;
	gobSalesChartsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QFont font;
	//font.setBold(true);
	//font.setPointSize(2);
	gobSalesChartsView->setFont(font);
	gobSalesChartsView->setLabelDetails(XLABEL, "Year/Month", font, QColor::fromRgb(0,0,0), Qt::AlignCenter);
	gobSalesChartsView->setLabelDetails(YLABEL, "Total", font, QColor::fromRgb(0, 0, 0), Qt::AlignCenter);
	gobSalesChartsView->setLabelDetails(HEADER, "Monthly Sales", font, QColor::fromRgb(0, 0, 0), Qt::AlignCenter);
	gobSalesChartsView->setGridLineStyle(Qt::DashLine);
	gobSalesChartsView->setGridColour(QColor::fromRgb(10, 40, 50));

	//All the bill amount vs date
	QStandardItemModel *model = new QStandardItemModel(this);
	QSqlQuery billQuery("SELECT SUM(amount) as total, year(date) as y, month(date) as m FROM bill WHERE deleted = 0 AND status = 1 GROUP BY MONTH(date)");
	int row = 0, col = 0;
	while (billQuery.next())
	{
		QString yearMonth = billQuery.value("y").toString() + "/" + billQuery.value("m").toString();
		QStandardItem* monthItem = new QStandardItem(yearMonth);
		model->setItem(row, col++, monthItem);
		QString total =  billQuery.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + yearMonth);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	gobSalesChartsView->setVerticalGridLines(true, row);
	gobSalesChartsView->setHorizontalGridLines(true, row);
	gobSalesChartsView->setModel(model);

	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	gobSalesChartsView->setSelectionModel(selectionModel);

	gobSalesChartsView->drawChart();
	gobSalesChartsView->setShowTotalRange();
	gobSalesChartsView->show();

	return gobSalesChartsView;
}
