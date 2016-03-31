#ifndef ES_ADDMANUAL_STOCKITEMS_H
#define ES_ADDMANUAL_STOCKITEMS_H
#include "ui_addmanualstockitems.h"
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESAddManualStockItems : public QWidget
{
	Q_OBJECT

public:
	ESAddManualStockItems(QWidget *parent = 0);
	~ESAddManualStockItems();

	public slots:
	void slotSearch();
	void slotItemSelected(int row, int col);
	void slotAddToStock();

private:
	Ui::AddManualStockItems ui;

};

#endif