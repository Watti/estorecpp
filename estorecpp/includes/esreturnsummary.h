#ifndef esreturnsummary_h__
#define esreturnsummary_h__

#include "ui_returnsummary.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "QSignalMapper"

class ESReturnSummary : public QWidget
{
	Q_OBJECT

public:
	ESReturnSummary(QWidget *parent = 0);
	~ESReturnSummary();

	Ui::ReturnSummaryWidget& getUI() { return ui; }

	public slots:
	void slotSearch();

private:
	Ui::ReturnSummaryWidget ui;
	QSignalMapper* m_removeButtonSignalMapper;

};

#endif // esreturnsummary_h__
