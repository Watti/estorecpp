#ifndef essinglepayment_h__
#define essinglepayment_h__

#include "ui_singlepayment.h"
#include <QtGui>

class ESSinglePayment : public QWidget
{
	Q_OBJECT

public:
	ESSinglePayment(QWidget *parent = 0);

	~ESSinglePayment();

	Ui::SinglePaymentWidget& getUI() { return ui; };

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected(QString);
	void slotCalculateBalance();
	void slotFinalizeBill();

private:
	bool validate();

	Ui::SinglePaymentWidget ui;
	QString m_customerId;
	QString m_selectedPM;

};


#endif // essinglepayment_h__
