#ifndef ES_MANAGESTOCKITEMS_H
#define ES_MANAGESTOCKITEMS_H
#include "ui_managestockitems.h"
#include <QStandardItemModel>
#include <QtGui>

class ESManageStockItems : public QWidget
{
	Q_OBJECT

public:
	ESManageStockItems(QWidget *parent = 0);

	~ESManageStockItems();

public slots:
	void slotUpdate();
	void slotRemove(QString itemId);
	void slotSearch(QString text);

private:

	void displayStockItems();
	Ui::ManageStockItems ui;
	QStandardItemModel* m_model;
	QSignalMapper m_buttonSignalMapper;

};

#endif