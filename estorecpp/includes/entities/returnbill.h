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
		struct ReturnItemInfo
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
		struct NewItemInfo
		{
			long stockId;
			QString itemCode;
			QString itemName;
			double itemPrice;
			double quantity;
			double discount;
		};

		ReturnBill();
		~ReturnBill();

		bool start();
		void end();

		bool isStarted() const { return m_started; }
		bool addReturnItem(QString oldBillId, QString itemCode);
		void addNewItem(QString stockId);
		void setInterest(QString interest);
		bool updateItemQuantity(long rowId, QString qtyStr, double& billedQty, double& returnPrice);
		bool updateNewItemQuantity(long rowId, QString qtyStr);

		void removeReturnItem(QString rowId);
		void removeNewItem(QString rowId);

		const std::map<int, ReturnItemInfo>& getReturnItemTable() const;
		const std::map<int, NewItemInfo>& getNewItemTable() const;

		double getSubTotal();
		double getTotal();

		void commit();
		void cancel();

		long getBillId() const { return m_billId; }
		long getOldBillId() const { return m_oldBillId; }

	private:
		void calculateTotal();
		
		std::map<int,ReturnItemInfo> m_returnItems;
		std::map<int,NewItemInfo> m_newItems;
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
