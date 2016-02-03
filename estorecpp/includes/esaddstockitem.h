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

	public slots:
	void slotAddStockItem();
	double getExistingQuantity() const;
	void setExistingQuantity(double val);

private:
	Ui::AddStockItem ui;
	double existingQuantity;
};

#endif