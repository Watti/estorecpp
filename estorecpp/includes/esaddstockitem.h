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
	double getExistingQuantityInMainStock() const;
	void setExistingQuantityInMainStock(double val);
	double getExistingQuantityInStock() const;
	void setExistingQuantityInStock(double val);
	bool isUpdate() const;
	void setUpdate(bool val);
	void setItemId(QString val);

private:
	Ui::AddStockItem ui;
	double m_existingQuantityInMainStock;
	double m_existingQuantityInStock;
	bool m_update;
	QString m_stockId;
};

#endif