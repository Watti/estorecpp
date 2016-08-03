#ifndef esstockreportcontainer_h__
#define esstockreportcontainer_h__

#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "ui_stockreportcontainer.h"
#include "esstockreport.h"
#include <memory>
#include "esitemwisesalessummary.h"

class ESStockReportContainer : public QWidget
{
	Q_OBJECT

public:
	ESStockReportContainer(QWidget *parent = 0);
	~ESStockReportContainer();

	Ui::StockReportContainerWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void onTabChanged(int tabIndex);

private:
	Ui::StockReportContainerWidget ui;
	ESStockReport* m_stockReport;
	ESItemWiseSalesSummary* m_itemWiseSalesReport;


};

#endif // esstockreportcontainer_h__
