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
	bool display = false;
	if (ES::Session::getInstance()->isSecondDisplayOn())
	{
		if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
			ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
		{
			display = true;
		}
	}
	else
	{
		display = true;
	}
	if (display)
	{
		m_revenueMasterSummary = new ESRevenueMasterReport(ui.tabWidget);
		ui.tabWidget->addTab(m_revenueMasterSummary, "Revenue Master Report");

		m_customerOutstandingSummary = new ESCustomerOutstandingSummary(ui.tabWidget);
		ui.tabWidget->addTab(m_customerOutstandingSummary, "Customer Outstanding");
	}
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
		if (!ES::Session::getInstance()->isSecondDisplayOn())
		{
			m_mdfSalesSummary->slotSearch();
		}
		else
		{
			m_revenueMasterSummary->slotSearch();
		}
		break;
	case 5:
		if (!ES::Session::getInstance()->isSecondDisplayOn())
		{
			m_revenueMasterSummary->slotSearch();
		}
		else
		{
			m_customerOutstandingSummary->slotSearch();
		}
		break;
	case 6:
		m_customerOutstandingSummary->slotSearch();
		break;
	default:
		break;
	}




}
