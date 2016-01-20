#ifndef ES_MANAGE_ORDERITEMS_H
#define ES_MANAGE_ORDERITEMS_H
#include "ui_manageorderitems.h"
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESManageOrderItems : public QWidget
{
	Q_OBJECT

public:
	ESManageOrderItems(QWidget *parent = 0);
	~ESManageOrderItems();

	public slots:
	void slotAddNewOrderItem();
	void slotSearch();

private:
	Ui::AddOrderItemWidget ui;

	void displayItems(QSqlQuery& queryItems);
};

#endif