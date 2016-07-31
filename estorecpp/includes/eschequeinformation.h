#ifndef eschequeinformation_h__
#define eschequeinformation_h__

#include "ui_chequeinformation.h"
#include <QtGui>
#include <memory>
#include "KDReportsReport.h"
#include "KDReportsHtmlElement.h"
#include "QPrinter"

class ESChequeInformation : public QWidget
{
	Q_OBJECT

public:
	ESChequeInformation(QWidget *parent = 0);
	~ESChequeInformation();

	Ui::ChequeInformationWidget& getUI() { return ui; }

	public slots:
	void slotSearch();
	void slotSetProcessed(QString rowId);
	void slotRevert(QString rowId);
	void slotPrev();
	void slotNext();
	void slotGenerateReport();
	void slotPrint(QPrinter* printer);

private:
	Ui::ChequeInformationWidget ui;
	QSignalMapper* m_processedButtonMapper;
	QSignalMapper* m_revertButtonMapper;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;
	std::shared_ptr<KDReports::Report> m_report;

};

#endif // eschequeinformation_h__
