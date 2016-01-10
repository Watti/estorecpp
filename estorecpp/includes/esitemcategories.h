#ifndef ES_ITEM_CATEGORIES_H
#define ES_ITEM_CATEGORIES_H
#include "ui_itemcategories.h"
#include <QStandardItemModel>
#include <QtGui>

class ESItemCategories : public QWidget
{
	Q_OBJECT

public:
	ESItemCategories(QWidget *parent = 0);
	~ESItemCategories();

	public slots:
	void slotAddNewCategory();
	void slotUpdate(QString itemCategoryId);
	void slotRemove(QString itemCategoryId);

private:
	Ui::ItemCategories ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;

	void displayStockItems();
};

#endif