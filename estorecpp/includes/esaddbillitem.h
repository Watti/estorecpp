#ifndef ADDBILLITEM_H
#define ADDBILLITEM_H
#include "ui_addbillitem.h"
#include <QKeyEvent>
#include <QtGui>

class ESAddBillItem : public QWidget
{
	Q_OBJECT

public:
	ESAddBillItem(QWidget *parent = 0);
	~ESAddBillItem();

	Ui::AddBillItemWidget& getUI() { return ui; };

	public slots:
	void slotShowAddItem();
	void slotAddToBill(QString itemId);
	void slotSearch();

signals:
	void enterPressedOnItem(QString itemId);

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	Ui::AddBillItemWidget ui;
	QSignalMapper* m_itemSignalMapper;

};

#endif
