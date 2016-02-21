#ifndef ES_CURRENTBILLS_H
#define ES_CURRENTBILLS_H
#include "ui_currentbills.h"
#include <QStandardItemModel>
#include <QtGui>

class ESCurrentBills : public QWidget
{
	Q_OBJECT
	enum BillStatus
	{
		UNDEFINED_BILL = 0,
		FINISHED_BILL,
		PENDING_BILL,
		SUSPENDED_BILL
	};

public:
	ESCurrentBills(QWidget *parent = 0);
	~ESCurrentBills();
	public slots:
	void slotTypeSelected();
	void slotProceed(QString);
private:
	Ui::CurrentBills ui;
	QString getStatusInString(BillStatus status) const;
	QSignalMapper* m_proceedButtonSignalMapper;

};

#endif