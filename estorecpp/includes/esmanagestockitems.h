#ifndef ES_MANAGESTOCKITEMS_H
#define ES_MANAGESTOCKITEMS_H
#include "ui_managestockitems.h"
#include <QStandardItemModel>
#include <QtGui>

class ESManageStockItems : public QWidget
{
	Q_OBJECT

public:
	typedef QStringList StockTableRow;
	ESManageStockItems(QWidget *parent = 0);

	~ESManageStockItems();

	public slots:
	void slotAddToStock();
	void slotUpdate(QString itemId);
	void slotRemove(QString itemId);
	void slotSearch();

private:
	//void addItemToStock(QString itemId);

	Ui::ManageStockItems ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	QSignalMapper* m_addToStockButtonSignalMapper;

};

#endif