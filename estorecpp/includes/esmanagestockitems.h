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
	void slotUpdate();
	void slotRemove(QString itemId);
	void slotSearch(QString text);
	void slotCombo(QString text);

private:
	Ui::ManageStockItems ui;
	QStandardItemModel* m_model;
	QSignalMapper m_buttonSignalMapper;

	void displayStockItems();
	void displayStockTableRow(StockTableRow row, QString itemId, bool inStock = true);

};

#endif