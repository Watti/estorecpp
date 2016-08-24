#include "includes/essalesreportcontainer.h"
#include "essalesummaryreport.h"
#include "esreturnsummary.h"
#include "espettycashsummary.h"
#include "esoveralllsalessummary.h"
#include "utility/session.h"
#include "utility/utility.h"
#include "esrevenuemasterreport.h"

ESSalesReportContainer::ESSalesReportContainer(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	m_salesSummary = new ESSalesSummary(ui.tabWidget);
	ui.tabWidget->addTab(m_salesSummary, "Sales Summary");

	m_returnSummary = new ESReturnSummary(ui.tabWidget);
	ui.tabWidget->addTab(m_returnSummary, "Return Summary");

	m_pettyCashSummary = new PettyCashSummary(ui.tabWidget);
	ui.tabWidget->addTab(m_pettyCashSummary, "Petty Cash Summary");

	m_overallSalesSummary = new OverallSalesSummary(ui.tabWidget);
	ui.tabWidget->addTab(m_overallSalesSummary, "Overall Sales Summary");

	if (!ES::Session::getInstance()->isSecondDisplayOn())
	{
		m_mdfSalesSummary = new MDFSalesSummary(ui.tabWidget);
		ui.tabWidget->addTab(m_mdfSalesSummary, "MDF Sales Summary");
	}

	m_revenueMasterSummary = new ESRevenueMasterReport(ui.tabWidget);
	ui.tabWidget->addTab(m_revenueMasterSummary, "Revenue Master Report");

	m_customerOutstandingSummary = new ESCustomerOutstandingSummary(ui.tabWidget);
	ui.tabWidget->addTab(m_customerOutstandingSummary, "Customer Outstanding");
	//onTabChanged(0);
}

ESSalesReportContainer::~ESSalesReportContainer()
{

}

void ESSalesReportContainer::slotSearch()
{

}

void ESSalesReportContainer::onTabChanged(int tabIndex)
{
	switch (tabIndex)
	{
	case 0:
		m_salesSummary->displayResults();
		break;
	case 1:
		m_returnSummary->slotSearch();
		break;
	case 2:
		m_pettyCashSummary->displayResults();
		break;
	case 3:
		m_overallSalesSummary->slotSearch();
		break;
	case 4:
		m_mdfSalesSummary->slotSearch();
		break;
	case 5:
		m_revenueMasterSummary->slotSearch();
		break;
	case 6:
		m_customerOutstandingSummary->slotSearch();
		break;
	default:
		break;
	}




}
