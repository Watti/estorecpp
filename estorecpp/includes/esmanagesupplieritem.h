#ifndef esmanagesupplieritem_h__
#define esmanagesupplieritem_h__
#include "ui_supplieritemwidget.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESManageSupplierItem : public QWidget
{
	Q_OBJECT

public:
	ESManageSupplierItem(QWidget *parent = 0);
	~ESManageSupplierItem();

	Ui::SupplierItemWidget& getUI() { return ui; };

public slots:
	void slotSearch();
	void slotItemDoubleClicked(int row, int col);

private:
	void displayItems(QSqlQuery& queryItems);

	Ui::SupplierItemWidget ui;
	bool m_update;
	QString m_orderId;

};

#endif // esmanagesupplieritem_h__
