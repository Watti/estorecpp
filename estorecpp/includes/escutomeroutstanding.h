#ifndef ES_CUSTOMER_OUTSTANDING_H
#define ES_CUSTOMER_OUTSTANDING_H
#include "ui_customeroutstanding.h"
#include <QStandardItemModel>
#include <QtGui>

class ESCustomerOutstanding : public QWidget
{
	Q_OBJECT

public:
	ESCustomerOutstanding(QWidget *parent = 0);
	~ESCustomerOutstanding();

	Ui::CustomerOutstandingWidget& getUI() { return ui; }

	public slots:
	void slotSearch();

private:
	Ui::CustomerOutstandingWidget ui;

};

#endif