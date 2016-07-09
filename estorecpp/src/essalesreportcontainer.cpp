#include "includes/essalesreportcontainer.h"
#include "essalesummaryreport.h"
#include "esreturnsummary.h"

ESSalesReportContainer::ESSalesReportContainer(QWidget *parent /*= 0*/) : QWidget(parent)
{
	ui.setupUi(this);

	ESSalesSummary* salesSummary = new ESSalesSummary(ui.tabWidget);
	ui.tabWidget->addTab(salesSummary, "Sales Summary");

	ESReturnSummary* returnSummary = new ESReturnSummary(ui.tabWidget);
	ui.tabWidget->addTab(returnSummary, "Return Summary");
}

ESSalesReportContainer::~ESSalesReportContainer()
{

}

void ESSalesReportContainer::slotSearch()
{

}
