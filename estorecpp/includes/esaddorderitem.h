#ifndef ES_ADD_ORDER_ITEM_H
#define ES_ADD_ORDER_ITEM_H
#include "ui_addorderitemwidget.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>

class AddOrderItem : public QWidget
{
	Q_OBJECT

public:
	AddOrderItem(QWidget *parent = 0);
	~AddOrderItem();

	Ui::AddOrderItemWidget& getUI() { return ui; };
	void setUpdate(bool update);
	QString getOrderId() const;
	void setOrderId(QString val);
	
	public slots:
	void slotPlaceNewOrder();
	void slotSearch();
	void slotSupplierSelected(int row, int col);
	void slotItemDoubleClicked(int row, int col);

private:
	void displayItems(QSqlQuery& queryItems);
	
	Ui::AddOrderItemWidget ui;
	bool m_update;
	QString m_orderId;
};

#endif