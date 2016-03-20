#ifndef SALES_REPORT_H
#define SALES_REPORT_H

#include "view/gobchartsview.h"
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

class ESSalesReport : public QWidget
{
	Q_OBJECT

public:
	ESSalesReport(QWidget *parent = 0);
	~ESSalesReport();

private:
	GobChartsView* m_gobChartsView;
	QAbstractItemModel* m_model;
	QItemSelectionModel* m_selectionModel;

};

#endif