#ifndef ES_UTILITY_H
#define ES_UTILITY_H

#include "QString"
#include "QWidget"


static const QString DEFAULT_DB_NUMERICAL_TO_DISPLAY = "0";
static const QString DEFAULT_DB_COMBO_VALUE = "select";
namespace ES
{
	class Utility
	{
	public:
		Utility();
		~Utility();
		static bool verifyUsingMessageBox(QWidget *parent, QString title, QString message);
		static float getOutstandingTotalFromSales(QString customerId);
		static float getTotalCreditOutstanding(QString customerId);
		static float getTotalChequeOutstanding(QString customerId);
		static float getOutstandingForBill(int billId);
		static void updateOutstandingAmount(QString customerId, double amount);
	private:

	};
}
#endif