#ifndef ESRETURNITEMWIDGET_H
#define ESRETURNITEMWIDGET_H
#include "ui_ReturnItems.h"
class ESReturnItems : public QWidget
{
	Q_OBJECT

public:
	ESReturnItems(QWidget *parent = 0);
	~ESReturnItems();
	Ui::ReturnItems& getUI();

	public slots:

private:
	Ui::ReturnItems ui;
};
#endif