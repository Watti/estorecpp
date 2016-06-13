#ifndef ES_MANAGE_ITEMS_H
#define ES_MANAGE_ITEMS_H
#include "ui_manageitems.h"
#include <QtSql/QSqlQuery>
#include <QtGui>

class ESManageItems : public QWidget
{
	Q_OBJECT

public:
	ESManageItems(QWidget *parent = 0);
	~ESManageItems();

	public slots:
	void slotAddNewItem();
	void slotUpdate(QString itemId);
	void slotRemove(QString itemId);
	void slotSearch();
	void slotFitImages();
	void slotHideImages();
	void slotPrev();
	void slotNext();

private:
	void displayItems(QSqlQuery& queryItems);

	Ui::ManageItems ui;
	QSignalMapper* m_updateButtonSignalMapper;
	QSignalMapper* m_removeButtonSignalMapper;
	bool m_imagesHidden;
	int m_pageOffset;
	int m_startingLimit;
	int m_totalRecords;
	int m_nextCounter;
	int m_maxNextCount;
};

#endif