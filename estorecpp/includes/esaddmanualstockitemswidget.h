#ifndef ES_ADDMANUAL_STOCKITEMS_H
#define ES_ADDMANUAL_STOCKITEMS_H
#include "ui_addmanualstockitems.h"
#include <QStandardItemModel>
#include <QtGui>

class ESAddManualStockItems : public QWidget
{
	Q_OBJECT

public:
	ESAddManualStockItems(QWidget *parent = 0);
	~ESAddManualStockItems();

	public slots:
	void slotSearch();

private:
	Ui::AddManualStockItems ui;

};

#endif