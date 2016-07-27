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

private:
	Ui::ChequeInformationWidget ui;

};

#endif // eschequeinformation_h__
