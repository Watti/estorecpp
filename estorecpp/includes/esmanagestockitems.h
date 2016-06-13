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
	void slotPrev();
	void slotNext();

private:
	//void addItemToStock(QString itemId);

	Ui::ManageStockItems ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	QSignalMapper* m_addToStockButtonSignalMapper;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;
};

#endif