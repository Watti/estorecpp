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
										  generateMonthlySummary();

	}
		break;
	case ESGenerateStatistics::ANNUAL:
	{
										 generateAnnualSummary();

	}
		break;
	case ESGenerateStatistics::DEMANDING_ITEMS:
	{
												  generateDemandingItemSummary();

	}
		break;
	default:
		break;
	}
}

QWidget* ESGenerateStatistics::generateChart(QStandardItemModel* model, const QString& titleText, GobChartsType chartType)
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

void ESGenerateStatistics::generateMonthlySummary()
{
	QStandardItemModel* model = new QStandardItemModel(this);
	int row = 0, col = 0;
	QString d = ui.fromDate->date().toString("yyyy-MM-dd");
	QString quaryStr = "SELECT SUM(amount) as total, YEAR(date) as y, MONTHNAME(date) as m FROM bill WHERE deleted = 0 AND  date BETWEEN '" +
		ui.fromDate->date().toString("yyyy-MM-dd") + "' AND '" + ui.toDate->date().toString("yyyy-MM-dd") + "' AND status = 1 GROUP BY MONTH(date)";
	QSqlQuery query(quaryStr);
	while (query.next())
	{
		QString yearMonth = query.value("y").toString() + "(" + query.value("m").toString() + ")";
		QStandardItem* monthItem = new QStandardItem(yearMonth);
		model->setItem(row, col++, monthItem);
		QString total = query.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + yearMonth);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}

	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Monthly Sales Summary");
	ui.chartGridLayout->addWidget(generateChart(model, title, BAR));
}

void ESGenerateStatistics::generateAnnualSummary()
{
	int  fromYear = ui.fromDate->date().year();
	int  toYear = ui.toDate->date().year();
	QStandardItemModel* model = new QStandardItemModel(this);
	int row = 0, col = 0;
	QString quaryStr = "SELECT SUM(amount) as total, YEAR(date) as y FROM bill WHERE deleted = 0 AND  YEAR(date) BETWEEN '" +
		QString::number(fromYear) + "' AND '" + QString::number(toYear) + "' AND status = 1 GROUP BY YEAR(date)";
	QSqlQuery query(quaryStr);
	while (query.next())
	{
		QString year = query.value("y").toString();
		QStandardItem* yearItem = new QStandardItem(year);
		model->setItem(row, col, yearItem);
		col++;

		QString total = query.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + year);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Annual Sales Summary");
	ui.chartGridLayout->addWidget(generateChart(model, title, PIE));
}

void ESGenerateStatistics::generateDemandingItemSummary()
{
	QStandardItemModel* model = new QStandardItemModel(this);
	int row = 0, col = 0;
	QString quaryStr = "SELECT SUM(sa.quantity) as qty , it.item_name as name  FROM sale as sa , stock as st , item as it WHERE (sa.stock_id = st.stock_id AND st.item_id = it.item_id) AND sa.date BETWEEN '" +
		ui.fromDate->date().toString("yyyy-MM-dd") + "' AND '" + ui.toDate->date().toString("yyyy-MM-dd") + "' AND sa.deleted = 0 GROUP BY  sa.stock_id ORDER BY SUM(sa.quantity) ASC LIMIT 5";
	QSqlQuery query(quaryStr);
	while (query.next())
	{
		QString name = query.value("name").toString();
		QStandardItem* yearItem = new QStandardItem(name);
		model->setItem(row, col, yearItem);
		col++;

		QString total = query.value("qty").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(total + " - " + name);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Demanding Items");
	ui.chartGridLayout->addWidget(generateChart(model, title, BAR));
}
