
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

	private:
		Session();
		static Session* m_session;
		User* m_user;
		bool m_billStarted;
	};

}
#endif