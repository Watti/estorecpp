#include "utility/session.h"
#include <crtdbg.h>
#include "utility/esmenumanager.h"

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

	Session::Session() : m_user(NULL), m_billStarted(false)
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

	void Session::startBill()
	{
		m_billStarted = true;
	}

	bool Session::isBillStarted() const
	{
		return m_billStarted;
	}

	void Session::endBill()
	{
		m_billStarted = false;
	}

	QString Session::getBillId() const
	{
		return m_billId;
	}

	void Session::setBillId(QString val)
	{
		m_billId = val;
	}

	void Session::setBackupPath(const QString& val)
	{
		m_dumpPath = val;
	}

	QString Session::getBackupPath() const
	{
		return m_dumpPath;
	}

	QString Session::getBranchName() const
	{
		return m_branchName;
	}

	void Session::setBranchName(const QString& val)
	{
		m_branchName = val;
	}

	void Session::invalidate()
	{
		QString perms = "";
		ES::MenuManager::instance()->reload(perms);

		delete m_user;
		m_user = NULL;
	}

}