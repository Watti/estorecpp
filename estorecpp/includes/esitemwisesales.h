#ifndef ES_ITEM_WISE_SALE_SUMMARY_REPORT_H
#define ES_ITEM_WISE_SALE_SUMMARY_REPORT_H

#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "ui_itemwisesales.h"
#include "KDReportsTableElement.h"
#include "qnamespace.h"
#include "QSignalMapper"
#include "QDateTime"

class ItemWiseSales : public QWidget
{
	Q_OBJECT

public:
	ItemWiseSales(QWidget *parent = 0);
	~ItemWiseSales();

	Ui::ItemWiseSales& getUI();
	void displayResults();

	public slots:
	void slotPrint(QPrinter* printer);
	void slotSearch();
	void slotGenerateReportForGivenItem(QString itemId);
	void slotGenerateReport();
	void slotPrev();
	void slotNext();
private:
	Ui::ItemWiseSales ui;

	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	KDReports::Report* report;
	QSignalMapper* m_generateReportSignalMapper;

	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;
};

#endif