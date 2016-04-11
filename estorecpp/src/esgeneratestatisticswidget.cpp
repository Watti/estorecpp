#include "esgeneratestatisticswidget.h"
#include "widget\gobchartswidget.h"
#include "utils\globalincludes.h"
#include "QDateTime"

ESGenerateStatistics::ESGenerateStatistics(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	ui.fromDate->setDate(QDate(2016, 05, 01));
	ui.toDate->setDate(QDate(2016, 06, 01));
	ui.statType->addItem("Daily", DAILY);
	ui.statType->addItem("Monthly", MONTHLY);
	ui.statType->addItem("Annual", ANNUAL);
	ui.statType->addItem("Items in Demand", DEMANDING_ITEMS);
	QObject::connect(ui.generateBtn, SIGNAL(clicked()), this, SLOT(slotGenerateReport()));
}

ESGenerateStatistics::~ESGenerateStatistics()
{
	ui.statType->currentIndex();
}

void ESGenerateStatistics::slotGenerateReport()
{
	QString fromDate = ui.fromDate->text();
	QString toDate = ui.toDate->text();

	int index = ui.statType->currentData().toInt();
	ChartType type = ChartType(index);

	switch (type)
	{
	case ESGenerateStatistics::DAILY:
		break;
	case ESGenerateStatistics::MONTHLY:
	{
										  QStandardItemModel model(this);
										  int row = 0, col = 0;
										  //int  fromMonth = ui.fromDate->date().month();
										  //int  toMonth = ui.toDate->date().month();

										  QString quaryStr = "SELECT SUM(amount) as total, YEAR(date) as y, MONTHNAME(date) as m FROM bill WHERE deleted = 0 AND  date BETWEEN '" + fromDate + "' AND '" + toDate + "' AND status = 1 GROUP BY MONTH(date)";
										  QSqlQuery query(quaryStr);
										  while (query.next())
										  {
											  QString yearMonth = query.value("y").toString() + "(" + query.value("m").toString() + ")";
											  QStandardItem* monthItem = new QStandardItem(yearMonth);
											  model.setItem(row, col++, monthItem);
											  QString total = query.value("total").toString();
											  QStandardItem* totalItem = new QStandardItem(total);
											  totalItem->setToolTip(total + " - " + yearMonth);
											  model.setItem(row, col, totalItem);
											  row++;
											  col = 0;
										  }
										  generateChart(&model, BAR);
	}
		break;
	case ESGenerateStatistics::ANNUAL:
	{
										 int  fromYear = ui.fromDate->date().year();
										 int  toYear = ui.toDate->date().year();
										 QStandardItemModel model(this);
										 int row = 0, col = 0;
										 QString quaryStr = "SELECT SUM(amount) as total, YEAR(date) as y FROM bill WHERE deleted = 0 AND  YEAR(date) BETWEEN '" + QString::number(fromYear) + "' AND '" + QString::number(toYear) + "' AND status = 1 GROUP BY YEAR(date)";
										 QSqlQuery query(quaryStr);
										 while (query.next())
										 {
											 QString year = query.value("y").toString();
											 QStandardItem* yearItem = new QStandardItem(year);
											 model.setItem(row, col, yearItem);
											 col++;

											 QString total = query.value("total").toString();
											 QStandardItem* totalItem = new QStandardItem(total);
											 totalItem->setToolTip(total + " - " + year);
											 model.setItem(row, col, totalItem);
											 row++;
											 col = 0;
										 }
										 generateChart(&model, PIE);
	}
		break;
	case ESGenerateStatistics::DEMANDING_ITEMS:
		break;
	default:
		break;
	}
}

QWidget* ESGenerateStatistics::generateChart(QStandardItemModel* model, GobChartsType chartType)
{
	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
	QLabel* title = new QLabel("Test");
	title->setAlignment(Qt::AlignCenter);
	GobChartsWidget* chartWidget = new GobChartsWidget(mainWidget);
	QAbstractItemModel* salesChartModel;
	QItemSelectionModel* salesSelectionModel;
	chartWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	QFont font;
	font.setBold(true);

	chartWidget->setFont(font);
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