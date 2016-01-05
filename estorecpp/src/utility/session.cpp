#include "utility/session.h"
#include <crtdbg.h>

namespace ES
{
	Session* Session::m_session = NULL;

	Session::~Session()
	{
		delete m_user;
	}

	Session* Session::getInstance()
	{
		if (!m_session)
		{
			m_session = new Session;
		}
		return m_session;
	}

	Session::Session() : m_user(NULL)
	{

	}

	User* Session::getUser() const
	{
		return m_user;
	}

	void Session::setUser(User* val)
	{
		m_user = val;
	}

}