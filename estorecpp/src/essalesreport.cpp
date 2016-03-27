#include "essalesreport.h"
#include "view/gobchartsfactory.h"
#include <QStandardItemModel>

ESSalesReport::ESSalesReport(QWidget *parent /* = 0 */)
:QWidget(parent)
{
	m_gobChartsView = GobChartsFactory::getInstance()->createChart(BAR, this);
	m_gobChartsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QStandardItemModel *model = new QStandardItemModel(this);
	model->setItem(0, 0, new QStandardItem(QString(" Apples ")));
	model->setItem(1, 0, new QStandardItem(QString(" Bananas ")));
	model->setItem(0, 1, new QStandardItem(QString(" 100 ")));
	model->setItem(1, 1, new QStandardItem(QString(" 60 ")));
	model->setItem(0, 2, new QStandardItem(QString(" 140 ")));
	model->setItem(1, 2, new QStandardItem(QString(" 60 ")));
	m_gobChartsView->setModel(model);

	QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
	m_gobChartsView->setSelectionModel(selectionModel);

	m_gobChartsView->drawChart();
	m_gobChartsView->show();
}

ESSalesReport::~ESSalesReport()
{}
