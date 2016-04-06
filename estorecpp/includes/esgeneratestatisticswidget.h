#ifndef	GENERATE_STATISTICS_H
#define GENERATE_STATISTICS_H

#include "ui_generatestatistics.h"
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESGenerateStatistics : public QWidget
{
	Q_OBJECT

public:
	ESGenerateStatistics(QWidget *parent = 0);
	~ESGenerateStatistics();

	public slots:


private:
	Ui::GenerateStatistics ui;

};

#endif 
