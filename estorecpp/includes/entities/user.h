#ifndef ES_USER_H
#define ES_USER_H

#include <QString>

namespace ES
{
	class User
	{
	public:
		enum UserType
		{
			UN_DEFINED = 0,
			DEV,
			MANAGER,
			STORE_KEEPER,
			CASHIER
		};
		User();
		~User();
		ES::User::UserType getType() const;
		void setType(ES::User::UserType val);
		QString getName() const;
		void setName(QString val);
	private:
		UserType m_type;
		QString m_name;
	};
}
#endif

