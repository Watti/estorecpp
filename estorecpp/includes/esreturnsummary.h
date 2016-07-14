#ifndef esreturnsummary_h__
#define esreturnsummary_h__

#include "ui_returnsummary.h"
#include <QStandardItemModel>
#include <QtSql/QSqlQuery>
#include <QtGui>
#include "QSignalMapper"
#include "KDReportsReport.h"
#include "qnamespace.h"
#include "KDReportsTableElement.h"

class ESReturnSummary : public QWidget
{
	Q_OBJECT

public:
	ESReturnSummary(QWidget *parent = 0);
	~ESReturnSummary();

	Ui::ReturnSummaryWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void slotDateChanged();
	void slotGenerateReport();
	void slotPrint(QPrinter* printer);

private:
	Ui::ReturnSummaryWidget ui;
	QSignalMapper* m_removeButtonSignalMapper;
	KDReports::Report report;
	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);

};

#endif // esreturnsummary_h__
