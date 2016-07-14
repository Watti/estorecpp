#ifndef ES_PETTY_CASH_SUMMARY_H
#define ES_PETTY_CASH_SUMMARY_H

#include "QPrinter"
#include "KDReportsReport.h"
#include "QSignalMapper"
#include "QWidget"
#include "ui_PettyCashSummary.h"
#include "KDReportsTableElement.h"

class PettyCashSummary : public QWidget
{
	Q_OBJECT
public:
	PettyCashSummary(QWidget *parent = 0);

	void displayResults();

	~PettyCashSummary();


	public slots:
	void slotDateChanged();
	void slotGenerateReportForGivenUser(QString);
	void slotPrint(QPrinter* printer);
	void slotGenerateReport();
private:
	Ui::PettyCashSummary ui;
	KDReports::Report report;
	QSignalMapper* m_generateReportSignalMapper;
	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);

};
#endif