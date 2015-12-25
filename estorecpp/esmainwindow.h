#ifndef ESMAINWINDOW_H
#define ESMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_esmainwindow.h"

class ESMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	ESMainWindow(QWidget *parent = 0);
	~ESMainWindow();

private:
	Ui::ESMainWindowClass ui;
};

#endif // ESMAINWINDOW_H
