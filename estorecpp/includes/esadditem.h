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

	public slots:
	void slotAddItem();
	bool isUpdate() const;
	void setUpdate(bool val);
	QString getItemId() const;
	void setItemId(QString val);

private:
	Ui::AddItemWidget ui;
	bool m_isUpdate;
	QString m_itemId;
};

#endif