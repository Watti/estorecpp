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
	ESManageSupplierItem(QString supplierId, QWidget *parent = 0);
	~ESManageSupplierItem();

	Ui::SupplierItemWidget& getUI() { return ui; };

public slots:
	void slotSearch();
	void slotItemDoubleClicked(int row, int col);
	void slotRemove(QString id);
	void slotAddSupplierItems();

private:
	void displayItems(QSqlQuery& queryItems);

	Ui::SupplierItemWidget ui;
	QSignalMapper* m_removeButtonSignalMapper;

	QString m_supplierId;

};

#endif // esmanagesupplieritem_h__
