#ifndef ES_OVERALL_SALE_SUMMARY_REPORT_H
#define ES_OVERALL_SALE_SUMMARY_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_overallsalessummary.h"
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"

class OverallSalesSummary : public QWidget
{
	Q_OBJECT

public:
	OverallSalesSummary(QWidget *parent = 0);
	~OverallSalesSummary();

	Ui::OverallSalesSummary& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReportForGivenUser(QString userId);
private:
	Ui::OverallSalesSummary ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report report;
	QSignalMapper* m_generateReportSignalMapper;
};

#endif