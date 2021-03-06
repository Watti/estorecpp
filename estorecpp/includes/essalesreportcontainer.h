#ifndef essalesreportcontainer_h__
#define essalesreportcontainer_h__

#include "ui_salesreport.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "espettycashsummary.h"
#include "esreturnsummary.h"
#include "essalesummaryreport.h"
#include "esoveralllsalessummary.h"
#include "esmdfsalessummary.h"
#include "esrevenuemasterreport.h"
#include "escustomeroutstandingsummary.h"
#include "escustomerwisesalessummary .h"
#include "esitemwisesales.h"
#include "billwisesalessummary.h"

class ESSalesReportContainer : public QWidget
{
	Q_OBJECT

public:
	ESSalesReportContainer(QWidget *parent = 0);
	~ESSalesReportContainer();

	Ui::SalesReportWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void onTabChanged(int tabIndex);

private:
	Ui::SalesReportWidget ui;
	PettyCashSummary* m_pettyCashSummary;
	ESReturnSummary* m_returnSummary; 
	ESSalesSummary* m_salesSummary;
	OverallSalesSummary* m_overallSalesSummary;
	MDFSalesSummary* m_mdfSalesSummary;
	ESRevenueMasterReport* m_revenueMasterSummary;
	ESCustomerOutstandingSummary* m_customerOutstandingSummary;
	ESCustomerWiseSalesSummary* m_customerWiseSalesSummary;
	ItemWiseSales* m_itemWiseSales;
	ESBillWiseSalesSummary* m_billWiseSalesSummary;
};

#endif // essalesreportcontainer_h__
