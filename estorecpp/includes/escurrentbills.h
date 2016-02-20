#ifndef ES_CURRENTBILLS_H
#define ES_CURRENTBILLS_H
#include "ui_currentbills.h"
#include <QStandardItemModel>
#include <QtGui>

class ESCurrentBills : public QWidget
{
	Q_OBJECT

public:
	ESCurrentBills(QWidget *parent = 0);
	~ESCurrentBills();
	
private:
	Ui::CurrentBills ui;

};

#endif