#ifndef ES_SALE_SUMMARY_REPORT_H
#define ES_SALE_SUMMARY_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_salesummary.h"
#include "KDReportsTableElement.h"
#include "qnamespace.h"

class ESSalesSummary : public QWidget
{
	Q_OBJECT

public:
	ESSalesSummary(QWidget *parent = 0);

	void displayResults();

	~ESSalesSummary();

	Ui::SaleSummary& getUI() { return ui; };

	public slots:
	void slotGenerate();
	void slotPrint(QPrinter* printer);
	void slotDateChanged();
private:
	Ui::SaleSummary ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report report;

};

#endif