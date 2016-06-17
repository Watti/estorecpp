#ifndef esfooter_h__
#define esfooter_h__

#include "ui_footer.h"
#include <QtGui>

class ESFooter : public QWidget
{
	Q_OBJECT

public:
	ESFooter(QWidget *parent = 0);
	~ESFooter();

	Ui::FooterWidget& getUI() { return ui; };

public slots:
	void textChanged();

private:
	Ui::FooterWidget ui;

};

#endif // esfooter_h__
