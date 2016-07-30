#ifndef eschequeinformation_h__
#define eschequeinformation_h__

#include "ui_chequeinformation.h"
#include <QtGui>

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

private:
	Ui::ChequeInformationWidget ui;
	QSignalMapper* m_processedButtonMapper;
	QSignalMapper* m_revertButtonMapper;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;

};

#endif // eschequeinformation_h__
