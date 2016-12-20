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
	void slotPrev();
	void slotNext();
	void slotRemoveItem(QString itemId);
private:
	Ui::ManageSuppliers ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	QSignalMapper* m_itemRemoveButtonSignalMapper;
	QString m_supplierId;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;

};

#endif