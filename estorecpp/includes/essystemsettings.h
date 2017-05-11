#ifndef essystemsettings_h__
#define essystemsettings_h__

#include "ui_systemsettingswidget.h"
#include <QtGui>

class ESSystemSettings : public QWidget
{
	Q_OBJECT

public:
	ESSystemSettings(QWidget *parent = 0);
	~ESSystemSettings();

	Ui::SystemSettings& getUI() { return ui; };

public slots:
	void resetDatabase();
	void clearBillSession();
private:
	Ui::SystemSettings ui;

};

#endif // essystemsettings_h__
