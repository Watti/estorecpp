#ifndef esmultiplepayment_h__
#define esmultiplepayment_h__

#include "ui_multiplepayment.h"
#include <QtGui>
#include "esaddbill.h"
#include "KDReportsTableElement.h"
#include "QPrinter"
#include "KDReportsReport.h"

class ESMultiplePayment : public QWidget
{
	Q_OBJECT

public:
	ESMultiplePayment(ESAddBill* addBill, QWidget *parent = 0, bool isReturnBill = false);

	~ESMultiplePayment();

	Ui::MultiplePaymentWidget& getUI() { return ui; };
	void setCustomerId(QString customerId);

	public slots:
	void slotSearch();
	void slotPaymentMethodSelected();
	void slotCalculateBalance();
	void slotAdd();
	void slotRemove(int row);
	void slotFinalizeBill();
	void slotPrint(QPrinter* printer);
	float getInitialNetAmount() const;
	void setInitialNetAmount(float val);
	float getOutstandingLimit() const;
	void setOutstandingLimit(float val);
	//void slotInterestChanged();

private:
	bool validate();
	void finishBill(double netAmount, int billId);
	void printBill(int billId, float total);
	void printRow(KDReports::TableElement& tableElement, int row, int col,
		QString elementStr, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	Ui::MultiplePaymentWidget ui;
	QString m_customerId;
	QString m_paymentType;
	QSignalMapper* m_removeButtonSignalMapper;
	ESAddBill* m_addBill;
	float m_initialNetAmount;
	//KDReports::Report report;
	bool m_isReturnBill;
	float m_outstandingLimit;
};

#endif // esmultiplepayment_h__
