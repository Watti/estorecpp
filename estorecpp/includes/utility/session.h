
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
	private:
		Session();
		static Session* m_session;
		User* m_user;
		bool m_billStarted;
		QString m_billId;
		QString m_dumpPath;
		QString m_branchName;
	};

}
#endif