#ifndef essinglepayment2_h__
#define essinglepayment2_h__

#include "ui_singlepayment.h"
#include <QtGui>
#include "esaddbill.h"
#include <KDReportsReport.h>
#include "QPrinter"
#include "KDReportsTableElement.h"
#include "esreturnitems.h"

class ESSinglePayment2 : public QWidget
{
	Q_OBJECT

public:
	ESSinglePayment2(ESReturnItems* returnItemsWidget, QWidget *parent = 0);

	~ESSinglePayment2();

	Ui::SinglePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);
	float getInitialNetAmount() const;
	void setInitialNetAmount(float val);

	float getReturnTotal() const;
	void setReturnTotal(float val);

	float getReturnInterest() const;
	void setReturnInterest(float val);
	public slots:
	void slotSearch();
	void slotPaymentMethodSelected();
	void slotCalculateBalance();
	void slotFinalizeBill();
	void slotInterestChanged();
	void slotPrint(QPrinter* printer);
	void slotDiscountPercentage();
	void slotPrintReturnBill();

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	void handleChequePayment(int billId, double netAmount);
	void handleCreditPayment(int billId, double netAmount);
	void handleCashPayment(int billId, double netAmount);
	void handleCreditCardPayment(int billId, double netAmount);
	void handleLoyaltyPayment(int billId, double netAmount);
	void finishBill(double netAmount, int billId);
	void printRow(KDReports::TableElement& tableElement, int row, int col, 
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	void printBill(int billId, float total);
	//float getTotalOutstanding(QString customerId);

	bool validate();
	Ui::SinglePaymentWidget ui;
	QString m_customerId;
	float m_initialNetAmount;
	QString m_paymentMethod;
	ESReturnItems* m_returnItemsWidget;
	float m_returnTotal;
	float m_returnStartAmount;
	float m_returnInterest;
	long m_billId;
	KDReports::Report report;
	QString m_returnBillId;
};


#endif // essinglepayment_h__
