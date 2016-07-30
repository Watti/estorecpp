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
		struct BillInfo
		{
			QString itemCode;
			QString itemName;
			double quantity;
			double itemPrice;
			double paidPrice;
			double returnPrice;
			QString date;
			double billedQuantity;
		};

		ReturnBill();
		~ReturnBill();

		bool start();
		void end();

		bool isStarted() const { return m_started; }
		bool addReturnItem(QString oldBillId, QString itemCode);
		void addNewItem();
		void setInterest(QString interest);
		bool updateItemQuantity(long rowId, QString qtyStr, double& billedQty, double& returnPrice);

		void removeReturnItem(QString rowId);
		void removeNewItem(QString rowId);

		const std::map<int, BillInfo>& getReturnItemTable() const;
		const std::map<int, BillInfo>& getNewItemTable() const;

		double getSubTotal();
		double getTotal();

		void commit();
		void cancel();

		long getBillId() const { return m_billId; }

	private:
		void calculateTotal();
		
		std::map<int,BillInfo> m_returnItems;
		std::map<int,BillInfo> m_newItems;
		bool m_started;
		long m_oldBillId;
		long m_billId;
		bool m_hasInterest;

		double m_interest;
		double m_subTotal;
		double m_total;

		long m_returnItemsIDGenerator;
		long m_newItemsIDGenerator;

	};
}

#endif // returnbill_h__
