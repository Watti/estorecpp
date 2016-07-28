#ifndef returnbill_h__
#define returnbill_h__

#include <QStringList>
#include <map>
#include <QTableWidget>

namespace ES
{
	class ReturnBill
	{
	public:
		ReturnBill();
		~ReturnBill();

		void start();
		void end();

		bool addReturnItem(QString oldBillId, QString itemCode);
		void addNewItem();
		void setInterest(QString interest);

		void removeReturnItem(QString rowId);
		void removeNewItem(QString rowId);

		const std::map<int, QStringList>& getReturnItemTable() const;
		const std::map<int, QStringList>& getNewItemTable() const;

		double getSubTotal();
		double getTotal();

		void commit();
		void cancel();

	private:
		void calculateTotal();
		
		std::map<int,QStringList> m_returnItems;
		std::map<int,QStringList> m_newItems;
		bool m_started;
		long m_oldBillId;
		long m_billId;
		bool m_hasInterest;

		double m_interest;
		double m_subTotal;
		double m_total;

	};
}

#endif // returnbill_h__
