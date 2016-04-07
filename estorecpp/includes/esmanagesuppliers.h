#ifndef ESMANAGESUPPLIERS_H
#define ESMANAGESUPPLIERS_H

#include "ui_managesupplierswidget.h"

class ESManageSuppliers : public QWidget
{
	Q_OBJECT

public:
	ESManageSuppliers(QWidget *parent = 0);
	~ESManageSuppliers();

public slots:
	void slotSearch();
	void slotAddSupplier();
	void slotShowAddSupplierView();
	void slotShowUpdateSupplierView();

private:
	Ui::ManageSuppliers ui;

};

#endif