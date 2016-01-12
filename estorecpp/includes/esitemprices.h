#ifndef ES_ITEM_PRICES_H
#define ES_ITEM_PRICES_H
#include "ui_itemprices.h"
#include <QStandardItemModel>
#include <QtGui>

class ESItemPrices : public QWidget
{
	Q_OBJECT

public:
	ESItemPrices(QWidget *parent = 0);
	~ESItemPrices();

	public slots:
	void slotAddNewItemPrice();
	void slotUpdate(QString itemPriceId);
	void slotRemove(QString itemPriceId);

private:
	Ui::ItemPrices ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;

	void displayStockItems();
};

#endif