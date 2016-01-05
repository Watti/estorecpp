#ifndef ES_MANAGESTOCKITEMS_H
#define ES_MANAGESTOCKITEMS_H
#include "ui_managestockitems.h"
#include <QStandardItemModel>

class ESManageStockItems : public QWidget
{
	Q_OBJECT

public:
	ESManageStockItems(QWidget *parent = 0);
	~ESManageStockItems();

public slots:
	void slotUpdate();
	void slotRemove();

private:
	Ui::ManageStockItems ui;
	QStandardItemModel* m_model;
};

#endif