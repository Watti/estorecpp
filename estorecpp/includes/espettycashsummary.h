#ifndef ES_PETTY_CASH_SUMMARY_H
#define ES_PETTY_CASH_SUMMARY_H

#include "QPrinter"
#include "KDReportsReport.h"
#include "QSignalMapper"
#include "QWidget"
#include "ui_PettyCashSummary.h"

class PettyCashSummary : public QWidget
{
	Q_OBJECT
public:
	PettyCashSummary(QWidget *parent = 0);
	~PettyCashSummary();


	public slots:

private:
	Ui::PettyCashSummary ui;

};
#endif