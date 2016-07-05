#ifndef ESRETURNITEMWIDGET_H
#define ESRETURNITEMWIDGET_H
#include "ui_ReturnItems.h"
#include "QPrinter"
#include "KDReportsReport.h"

class ESReturnItems : public QWidget
{
	Q_OBJECT

public:
	ESReturnItems(QWidget *parent = 0);
	~ESReturnItems();
	Ui::ReturnItems& getUI();

	public slots:
	void slotSelect();

	void slotAddReturnedItem();
	void slotPrint(QPrinter* printer);
private:
	void printReturnItemInfo();

	Ui::ReturnItems ui;
	//KDReports::Report report;
};

#endif