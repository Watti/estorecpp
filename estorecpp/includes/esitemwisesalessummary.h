#ifndef eitemwisesalessummary_h__
#define eitemwisesalessummary_h__

#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "QSignalMapper"
#include "KDReportsReport.h"
#include "qnamespace.h"
#include "KDReportsTableElement.h"
#include "ui_salessummarybyitems.h"

class ESItemWiseSalesSummary : public QWidget
{
	Q_OBJECT

public:
	ESItemWiseSalesSummary(QWidget *parent = 0);
	~ESItemWiseSalesSummary();

	Ui::ItemWiseSalesWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void slotDateChanged();
	void slotGenerateReport();
	void slotPrint(QPrinter* printer);

private:
	Ui::ItemWiseSalesWidget ui;
	KDReports::Report report;
};

#endif // eitemwisesalessummary_h__
