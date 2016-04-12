#include "esgeneratestatisticswidget.h"
#include "widget\gobchartswidget.h"
#include "utils\globalincludes.h"
#include "QDateTime"

ESGenerateStatistics::ESGenerateStatistics(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	QString fromDateStr("2016-05-01");
	QString toDateStr("2016-06-01");
	ui.fromDate->setDate(QDate::fromString(fromDateStr, "yyyy-MM-dd"));
	ui.toDate->setDate(QDate::fromString(toDateStr, "yyyy-MM-dd"));
	ui.fromDate->setDisplayFormat("yyyy-MM-dd");
	ui.toDate->setDisplayFormat("yyyy-MM-dd");
	ui.statType->addItem("Daily", DAILY);
	ui.statType->addItem("Monthly", MONTHLY);
	ui.statType->addItem("Annual", ANNUAL);
	ui.statType->addItem("Top Selling Items", TOP_SELLING_ITEMS);
	ui.statType->addItem("Least Selling Items", LEAST_SELLING_ITEM);
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
		generateDailySummary();
		break;
	case ESGenerateStatistics::MONTHLY:
		generateMonthlySummary();
		break;
	case ESGenerateStatistics::ANNUAL:
		generateAnnualSummary();
	case ESGenerateStatistics::TOP_SELLING_ITEMS:
		generateDemandingItemSummary();
		break;
	case ESGenerateStatistics::LEAST_SELLING_ITEM:
		generateLeastSellingItemsSummary();
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
		totalItem->setToolTip(yearMonth + " - " + total);
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
		totalItem->setToolTip(year + " - " + total);
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
		ui.fromDate->date().toString("yyyy-MM-dd") + "' AND '" + ui.toDate->date().toString("yyyy-MM-dd") + "' AND sa.deleted = 0 GROUP BY  sa.stock_id ORDER BY SUM(sa.quantity) ASC LIMIT 10";
	QSqlQuery query(quaryStr);
	while (query.next())
	{
		QString itemName = query.value("name").toString();
		QStandardItem* item = new QStandardItem(itemName);
		model->setItem(row, col, item);
		col++;

		QString total = query.value("qty").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(itemName + " - " + total);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Top Selling Items");
	ui.chartGridLayout->addWidget(generateChart(model, title, BAR));
}

void ESGenerateStatistics::generateDailySummary()
{
	//ONLY SHOW LATEST 10 DAYS RESULTS
	QStandardItemModel* model = new QStandardItemModel(this);
	int row = 0, col = 0;
	QString quaryStr = "SELECT SUM(amount) as total, DATE(date) as d FROM bill WHERE deleted = 0 AND  date BETWEEN '" +
		ui.fromDate->date().toString("yyyy-MM-dd") + "' AND '" + ui.toDate->date().toString("yyyy-MM-dd") + "' AND status = 1 GROUP BY  DATE(date) ORDER BY DATE(date) DESC LIMIT 10";
	QSqlQuery
		query(quaryStr);
	while (query.next())
	{
		QString date = query.value("d").toString();
		QStandardItem* dateItem = new QStandardItem(date);
		model->setItem(row, col++, dateItem);
		QString total = query.value("total").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(date + " - " + total);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}

	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Daily Sales Summary");
	ui.chartGridLayout->addWidget(generateChart(model, title, BAR));
}

void ESGenerateStatistics::generateLeastSellingItemsSummary()
{

	QStandardItemModel* model = new QStandardItemModel(this);
	int row = 0, col = 0;
	QString quaryStr = "SELECT SUM(sa.quantity) as qty , it.item_name as name  FROM sale as sa , stock as st , item as it WHERE (sa.stock_id = st.stock_id AND st.item_id = it.item_id) AND sa.date BETWEEN '" +
		ui.fromDate->date().toString("yyyy-MM-dd") + "' AND '" + ui.toDate->date().toString("yyyy-MM-dd") + "' AND sa.deleted = 0 GROUP BY  sa.stock_id ORDER BY SUM(sa.quantity) DESC LIMIT 10";
	QSqlQuery query(quaryStr);
	while (query.next())
	{
		QString itemName = query.value("name").toString();
		QStandardItem* item = new QStandardItem(itemName);
		model->setItem(row, col, item);
		col++;

		QString total = query.value("qty").toString();
		QStandardItem* totalItem = new QStandardItem(total);
		totalItem->setToolTip(itemName + " - " + total);
		model->setItem(row, col, totalItem);
		row++;
		col = 0;
	}
	QLayoutItem *child;
	while ((child = ui.chartGridLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}
	QString title("Least Selling Items");
	ui.chartGridLayout->addWidget(generateChart(model, title, BAR));
}
