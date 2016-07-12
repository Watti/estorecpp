#ifndef essalesreportcontainer_h__
#define essalesreportcontainer_h__

#include "ui_salesreport.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESSalesReportContainer : public QWidget
{
	Q_OBJECT

public:
	ESSalesReportContainer(QWidget *parent = 0);
	~ESSalesReportContainer();

	Ui::SalesReportWidget& getUI() { return ui; }

	public slots:
	void slotSearch();

private:
	Ui::SalesReportWidget ui;

};

#endif // essalesreportcontainer_h__
