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
private:
	void printRow(KDReports::TableElement& tableElement, int row, int col, 
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);

	Ui::CurrentBills ui;
	QSignalMapper* m_proceedButtonSignalMapper;
	QSignalMapper* m_voidBillButtonSignalMapper;
	QSignalMapper* m_reprintBillButtonSignalMapper;

};

#endif