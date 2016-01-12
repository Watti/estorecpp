#ifndef ES_ADD_ITEM_PRICE_H
#define ES_ADD_ITEM_PRICE_H

#include "ui_additempricewidget.h"
#include <QStandardItemModel>
#include <QtGui>

class AddItemPrice : public QWidget
{
	Q_OBJECT

public:
	AddItemPrice(QWidget *parent = 0);
	~AddItemPrice();

	Ui::AddItemPriceWidget& getUI() { return ui; };

	public slots:
	void slotAddItemPrice();

private:
	Ui::AddItemPriceWidget ui;
};

#endif