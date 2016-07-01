#ifndef ES_SALE_SUMMARY_REPORT_H
#define ES_SALE_SUMMARY_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_salesummary.h"

class ESSalesSummary : public QWidget
{
	Q_OBJECT

public:
	ESSalesSummary(QWidget *parent = 0);
	~ESSalesSummary();

	Ui::SaleSummary& getUI() { return ui; };

	public slots:
	void slotGenerate();
	void slotPrint(QPrinter* printer);

private:
	Ui::SaleSummary ui;
	KDReports::Report report;

};

#endif