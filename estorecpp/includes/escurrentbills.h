#ifndef ES_CURRENTBILLS_H
#define ES_CURRENTBILLS_H
#include "ui_currentbills.h"
#include <QStandardItemModel>
#include <QtGui>
#include "QSignalMapper"

class ESCurrentBills : public QWidget
{
	Q_OBJECT

public:
	ESCurrentBills(QWidget *parent = 0);
	~ESCurrentBills();
	
	public slots:
	void slotSearch();
	void slotProceed(QString);

private:
	Ui::CurrentBills ui;
	QSignalMapper* m_proceedButtonSignalMapper;

};

#endif