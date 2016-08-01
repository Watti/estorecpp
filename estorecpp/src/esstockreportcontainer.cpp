#include "includes/esstockreportcontainer.h"
#include "essalesummaryreport.h"
#include "esreturnsummary.h"
#include "espettycashsummary.h"
#include "esoveralllsalessummary.h"

ESStockReportContainer::ESStockReportContainer(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	m_stockReport = new ESStockReport(ui.tabWidget);
 	ui.tabWidget->addTab(m_stockReport, "Stock Re-Order Report");
	
	onTabChanged(0);
}

ESStockReportContainer::~ESStockReportContainer()
{

}

void ESStockReportContainer::slotSearch()
{

}

void ESStockReportContainer::onTabChanged(int tabIndex)
{
	switch (tabIndex)
	{
	case 0:
		m_stockReport->displayResults();
		break;
	case 1:
		/*m_returnSummary->slotSearch();*/
		break;
	case 2:
		/*m_pettyCashSummary->displayResults();*/
		break;
	case 3:
		/*m_overallSalesSummary->slotSearch();*/
		break;
	default:
		break;
	}




}
