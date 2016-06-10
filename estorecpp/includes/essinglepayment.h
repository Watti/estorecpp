#ifndef essinglepayment_h__
#define essinglepayment_h__

#include "ui_singlepayment.h"
#include <QtGui>
#include "esaddbill.h"
#include <KDReportsReport.h>
#include "QPrinter"
#include "KDReportsTableElement.h"

class ESSinglePayment : public QWidget
{
	Q_OBJECT

public:
	ESSinglePayment(ESAddBill* addBill, QWidget *parent = 0);

	~ESSinglePayment();

	Ui::SinglePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected();
	void slotCalculateBalance();
	void slotFinalizeBill();
	void slotInterestChanged();
	void slotPrint(QPrinter* printer);

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	void handleChequePayment(int billId, double netAmount);
	void handleCreditPayment(int billId, double netAmount);
	void handleCashPayment(int billId, double netAmount);
	void handleCreditCardPayment(int billId, double netAmount);
	void finishBill(double netAmount, int billId);
	void printRow(KDReports::TableElement& tableElement, int row, int col, QString elementStr);
	void printBill(int billId, float total);

	bool validate();

	Ui::SinglePaymentWidget ui;
	QString m_customerId;
	ESAddBill* m_addBill;
	
	//KDReports::Report report;

};


#endif // essinglepayment_h__
