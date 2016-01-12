#ifndef ES_ADD_ITEM_CATEGORY_H
#define ES_ADD_ITEM_CATEGORY_H

#include "ui_additemcategorywidget.h"
#include <QStandardItemModel>
#include <QtGui>

class AddItemCategory : public QWidget
{
	Q_OBJECT

public:
	AddItemCategory(QWidget *parent = 0);
	~AddItemCategory();

	Ui::AddItemCategoryWidget& getUI() { return ui; };

	public slots:
	void slotAddItemCategory();

private:
	Ui::AddItemCategoryWidget ui;
};

#endif