#include "entities/user.h"

namespace ES
{
	User::User() : m_type(User::UN_DEFINED)
	{
	}
	
	User::~User()
	{
	}

	User::UserType User::getType() const
	{
		return m_type;
	}

	void User::setType(UserType val)
	{
		m_type = val;
	}

	QString User::getName() const
	{
		return m_name;
	}

	void User::setName(QString val)
	{
		m_name = val;
	}

}
