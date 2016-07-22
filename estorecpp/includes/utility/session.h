
#ifndef ES_SESSION_H
#define ES_SESSION_H

#include "entities\user.h"

namespace ES
{
	class Session
	{
	public:
		~Session();
		static Session* getInstance();

		User* getUser() const;
		void setUser(User* val);
		void startBill();
		bool isBillStarted() const;
		void endBill();
		QString getBillId() const;
		void setBillId(QString val);
		QString getBackupPath() const;
		void setBackupPath(const QString& val);
		QString getBranchName() const;
		void setBranchName(const QString& val);
		void invalidate();
		void setLowerPriceBlocked(bool blocked);
		bool isLowerPriceBlocked() const;
		QString getServerIP() const;
		void setServerIP(QString val);
		QString getBillTitle() const;
		void setBillTitle(QString val);
		QString getBillAddress() const;
		void setBillAddress(QString val);
		QString getBillPhone() const;
		void setBillPhone(QString val);
		int getBillItemFontSize() const;
		void setBillItemFontSize(int val);
		QString getBillEmail() const;
		void setBillEmail(QString val);
		bool isSecondDisplayOn() const;
		void setSecondDisplayOn(bool val);
	private:
		Session();
		static Session* m_session;
		User* m_user;
		bool m_billStarted;
		QString m_billId;
		QString m_dumpPath;
		QString m_branchName;
		QString m_serverIP;
		bool m_lowerPriceBlocked;
		QString m_billTitle;
		QString m_billAddress;
		QString m_billPhone;
		int m_billItemFontSize;
		QString m_billEmail;
		bool m_secondDisplayOn;
	};

}
#endif