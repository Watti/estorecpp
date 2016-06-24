#ifndef essecondarydisplay_h__
#define essecondarydisplay_h__

#include "ui_secondarydisplaywidget.h"
#include <QtGui>

class ESSecondaryDisplay : public QWidget
{
	Q_OBJECT

public:
	ESSecondaryDisplay(QWidget *parent = 0);
	~ESSecondaryDisplay();

	Ui::SecondaryDisplayWidget& getUI() { return ui; }

	void startBill();
	void endBill();

	public slots:
	void slotSearch();

private:
	Ui::SecondaryDisplayWidget ui;

};

#endif // essecondarydisplay_h__
