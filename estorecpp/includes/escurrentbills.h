#ifndef ES_CURRENTBILLS_H
#define ES_CURRENTBILLS_H
#include "ui_currentbills.h"
#include <QStandardItemModel>
#include <QtGui>
#include "QSignalMapper"
#include "KDReportsTableElement.h"
#include "KDReportsReport.h"

class ESCurrentBills : public QWidget
{
	Q_OBJECT

public:
	ESCurrentBills(QWidget *parent = 0);
	~ESCurrentBills();
	
	public slots:
	void slotSearch();
	void slotProceed(QString);
	void slotVoidBill(QString);
	void slotReprint(QString);
	void slotInvisible(QString);
	void slotPrint(QPrinter* printer);
	void slotPrev();
	void slotNext();

private:
	void printRow(KDReports::TableElement& tableElement, int row, int col, 
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	float getTotalOutstanding(QString customerId);
	float getOutstandingForBill(int billId);
	Ui::CurrentBills ui;
	QSignalMapper* m_proceedButtonSignalMapper;
	QSignalMapper* m_voidBillButtonSignalMapper;
	QSignalMapper* m_reprintBillButtonSignalMapper;
	QSignalMapper* m_invisibleButtonSignalMapper;
	KDReports::Report report;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;
};

#endif