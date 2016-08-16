#ifndef ES_MDF_SALE_SUMMARY_REPORT_H
#define ES_MDF_SALE_SUMMARY_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_mdfsalessummary.h"
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"

class MDFSalesSummary : public QWidget
{
	Q_OBJECT

public:
	MDFSalesSummary(QWidget *parent = 0);
	~MDFSalesSummary();

	Ui::MDFSalesSummary& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReportForGivenUser(QString userId);
	void slotGenerateReport();
private:
	Ui::MDFSalesSummary ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report report;
	QSignalMapper* m_generateReportSignalMapper;
};

#endif