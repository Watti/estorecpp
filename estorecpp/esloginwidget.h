#ifndef ESLOGINWIDGET_H
#define ESLOGINWIDGET_H

#include <QtWidgets/QWidget>
#include "ui_loginwidget.h"

class ESLoginWidget : public QWidget
{
	Q_OBJECT

public:
	ESLoginWidget(QWidget *parent = 0);
	~ESLoginWidget();

public slots:
	void slotLogin();

private:
	Ui::LoginWidget ui;
};

#endif // ESLOGINWIDGET_H