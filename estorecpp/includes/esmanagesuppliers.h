#ifndef ESMANAGESUPPLIERS_H
#define ESMANAGESUPPLIERS_H

#include "ui_managesupplierswidget.h"
#include <QtGui>

class ESManageSuppliers : public QWidget
{
	Q_OBJECT

public:
	ESManageSuppliers(QWidget *parent = 0);
	~ESManageSuppliers();

public slots:
	void slotSearch();
	void slotAddSupplier();
	void slotUpdateSupplier();
	void slotShowAddSupplierView();
	void slotShowAddSupplierItemView();
	void slotUpdate(QString id);
	void slotRemove(QString id);
	//void slotRemoveItem(QString supplierId, QString itemId);

private:
	Ui::ManageSuppliers ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	//QSignalMapper* m_itemRemoveButtonSignalMapper;

	QString m_supplierId;

};

#endif