#include "includes/essalesreportcontainer.h"
#include "essalesummaryreport.h"
#include "esreturnsummary.h"
#include "espettycashsummary.h"
#include "esoveralllsalessummary.h"

ESSalesReportContainer::ESSalesReportContainer(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	ESSalesSummary* salesSummary = new ESSalesSummary(ui.tabWidget);
	ui.tabWidget->addTab(salesSummary, "Sales Summary");

	ESReturnSummary* returnSummary = new ESReturnSummary(ui.tabWidget);
	ui.tabWidget->addTab(returnSummary, "Return Summary");

	PettyCashSummary* pettyCashSummary = new PettyCashSummary(ui.tabWidget);
	ui.tabWidget->addTab(pettyCashSummary, "Petty Cash Summary");

	OverallSalesSummary* overallSalesSummary = new OverallSalesSummary(ui.tabWidget);
	ui.tabWidget->addTab(overallSalesSummary, "Overall Sales Summary");
}

ESSalesReportContainer::~ESSalesReportContainer()
{

}

void ESSalesReportContainer::slotSearch()
{

}
