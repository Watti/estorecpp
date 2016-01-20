#ifndef ES_ADD_ORDER_ITEM_H
#define ES_ADD_ORDER_ITEM_H
#include "ui_addorderitemwidget.h"
#include <QStandardItemModel>
#include <QtGui>

class AddOrderItem : public QWidget
{
	Q_OBJECT

public:
	AddOrderItem(QWidget *parent = 0);
	~AddOrderItem();

	Ui::AddOrderItemWidget& getUI() { return ui; };
	void setUpdate(bool update);
	
	public slots:
	void slotAddOrderItem();

private:
	Ui::AddOrderItemWidget ui;
	bool m_update;

};

#endif