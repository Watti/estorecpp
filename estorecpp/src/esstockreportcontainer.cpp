#include "includes/esstockreportcontainer.h"
#include "essalesummaryreport.h"
#include "esreturnsummary.h"
#include "espettycashsummary.h"
#include "esoveralllsalessummary.h"
#include "utility/session.h"
#include "entities/user.h"

ESStockReportContainer::ESStockReportContainer(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

	m_stockReport = new ESStockReport(ui.tabWidget);
	ui.tabWidget->addTab(m_stockReport, "Stock Re-Order Report");

	if (ES::Session::getInstance()->isSecondDisplayOn())
	{
		if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
			ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
		{
			m_itemWiseSalesReport = new ESItemWiseSalesSummary(ui.tabWidget);
			ui.tabWidget->addTab(m_itemWiseSalesReport, "Sold Items Summary");
		}
	}
	else
	{
		m_itemWiseSalesReport = new ESItemWiseSalesSummary(ui.tabWidget);
		ui.tabWidget->addTab(m_itemWiseSalesReport, "Sold Items Summary");
	}
	m_overallStockItemReport = new ESOverallStockItemReport(ui.tabWidget);
	ui.tabWidget->addTab(m_overallStockItemReport, "Stock Item Summary");
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
		if (ES::Session::getInstance()->isSecondDisplayOn())
		{
			if (ES::Session::getInstance()->getUser()->getType() == ES::User::SENIOR_MANAGER ||
				ES::Session::getInstance()->getUser()->getType() == ES::User::DEV)
			{
				m_itemWiseSalesReport->slotSearch();
			}
		}
		else
		{
			m_itemWiseSalesReport->slotSearch();
		}
		break;
	case 2:
		m_overallStockItemReport->displayResults();
		break;
	case 3:
		/*m_overallSalesSummary->slotSearch();*/
		break;
	default:
		break;
	}




}
