#ifndef	GENERATE_STATISTICS_H
#define GENERATE_STATISTICS_H

#include "ui_generatestatistics.h"
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "QVector"
#include "QStandardItemModel"
#include "utils/globalincludes.h"

class ESGenerateStatistics : public QWidget
{
	Q_OBJECT

public:
	enum  ChartType
	{
		DAILY = 1,
		MONTHLY,
		ANNUAL,
		DEMANDING_ITEMS
	};
	ESGenerateStatistics(QWidget *parent = 0);
	~ESGenerateStatistics();

	public slots:
	void slotGenerateReport();


private:
	Ui::GenerateStatistics ui;
	QWidget* generateChart(QStandardItemModel* model, GobChartsType chartType = BAR);

};

#endif 
