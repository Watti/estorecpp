#ifndef ES_MANAGE_ITEMS_H
#define ES_MANAGE_ITEMS_H
#include "ui_manageitems.h"
#include <QStandardItemModel>
#include <QtGui>

class ESManageItems : public QWidget
{
	Q_OBJECT

public:
	ESManageItems(QWidget *parent = 0);
	~ESManageItems();

	public slots:
	void slotAddNewItem();
	void slotUpdate(QString itemCategoryId);
	void slotRemove(QString itemCategoryId);

private:
	Ui::ManageItems ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;

	void displayStockItems();
};

#endif