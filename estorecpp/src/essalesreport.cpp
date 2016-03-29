#include "essalesreport.h"
#include "view/gobchartsfactory.h"
#include <QStandardItemModel>
#include "QSqlQuery"

ESSalesReport::ESSalesReport(QWidget *parent /* = 0 */)
:QWidget(parent)
{
	m_gobChartsView = GobChartsFactory::getInstance()->createChart(BAR, this);
	m_gobChartsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//All the bill amount vs date
	QStandardItemModel *model = new QStandardItemModel(this);
	QSqlQuery billQuery("SELECT date, amount FROM bill where deleted = 0 AND status = 1");
	int row = 0, col = 0;
	while (billQuery.next())
	{
		QStandardItem* dateItem = new QStandardItem(billQuery.value("date").toString());
		model->setItem(row, col++, dateItem); 
		QStandardItem* stdItem = new QStandardItem(billQuery.value("amount").toString());
		stdItem->setToolTip(billQuery.value("amount").toString());
		model->setItem(row, col, stdItem);
		//model->setVerticalHeaderItem(row, dateItem);
		row++;
		col = 0;
	}
	m_gobChartsView->setModel(model);

	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	m_gobChartsView->setSelectionModel(selectionModel);

	m_gobChartsView->drawChart();
	m_gobChartsView->setShowTotalRange();
	m_gobChartsView->show();
}

ESSalesReport::~ESSalesReport()
{

}
