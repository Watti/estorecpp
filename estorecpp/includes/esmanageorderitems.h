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
	void slotUpdate(QString itemId);
	void slotRemove(QString itemId);
	void slotPrev();
	void slotNext();

private:
	Ui::ManageOrderItemWidget ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;

	void displayItems(QSqlQuery& queryItems);
};

#endif