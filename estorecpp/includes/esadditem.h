#ifndef ES_ADD_ITEM_H
#define ES_ADD_ITEM_H

#include "ui_additemwidget.h"
#include <QStandardItemModel>
#include <QtGui>

class AddItem : public QWidget
{
	Q_OBJECT

public:
	AddItem(QWidget *parent = 0);
	~AddItem();

	Ui::AddItemWidget& getUI() { return ui; };

	bool isUpdate() const;
	void setUpdate(bool val);
	QString getItemId() const;
	void setItemId(QString val);
	void setItemCategoryId(QString categoryId);

	public slots:
	void slotAddItem();
	void slotCategorySelected(int row, int col);
	void slotAddImage();
	void slotSearch();

private:
	Ui::AddItemWidget ui;
	bool m_isUpdate;
	QString m_itemId, m_categoryId;
	QString m_itemImage;

};

#endif