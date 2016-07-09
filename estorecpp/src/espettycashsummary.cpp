#include "espettycashsummary.h"

PettyCashSummary::PettyCashSummary(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	QStringList userWiseLabels;
	userWiseLabels.append("USER");
	userWiseLabels.append("INCOME");
	userWiseLabels.append("EXPENSE");
	userWiseLabels.append("BALANCE");

}


PettyCashSummary::~PettyCashSummary()
{

}
