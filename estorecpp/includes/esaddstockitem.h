#ifndef ES_ADD_STOCK_ITEM_H
#define ES_ADD_STOCK_ITEM_H
#include "ui_addstockitem.h"
#include <QStandardItemModel>
#include <QtGui>

class AddStockItem : public QWidget
{
	Q_OBJECT

public:
	AddStockItem(QWidget *parent = 0);
	~AddStockItem();

	Ui::AddStockItem& getUI() { return ui; };

	void addToPriceMap(QString key, QString value);
	public slots:
	void slotAddStockItem();

private:
	Ui::AddStockItem ui;
	QMap<QString, QString> m_priceMap;
};

#endif