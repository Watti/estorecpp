#ifndef eslatepayment_h__
#define eslatepayment_h__

#include "ui_latepaymentwidget.h"
#include <QtGui>
#include "KDReportsReport.h"

class ESLatePayment : public QWidget
{
	Q_OBJECT
public:
	ESLatePayment(QWidget *parent = 0);
	~ESLatePayment();

	Ui::LatePaymentWidget& getUI() { return ui; }
	void setCustomerId(QString customerId);

public slots:
	void slotOk();
	void slotCalculateCashBalance();
	void slotCalculateChequeBalance();
	void slotPrint(QPrinter* printer);

private:
	Ui::LatePaymentWidget ui;
	QString m_customerId;
	KDReports::Report report;

};

#endif // eslatepayment_h__
