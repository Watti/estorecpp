#include "utility/session.h"
#include <crtdbg.h>
#include "utility/esmenumanager.h"
#include "utility/esmainwindowholder.h"
#include "essecondarydisplay.h"

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

	Session::Session() : m_user(NULL), m_billStarted(false), m_lowerPriceBlocked(true)
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

		ES::MainWindowHolder::instance()->getSecondaryDisplay()->startBill();
	}

	bool Session::isBillStarted() const
	{
		return m_billStarted;
	}

	void Session::endBill()
	{
		m_billStarted = false;
		ES::MainWindowHolder::instance()->getSecondaryDisplay()->endBill();
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

	void Session::setLowerPriceBlocked(bool blocked)
	{
		m_lowerPriceBlocked = blocked;
	}
	
	bool Session::isLowerPriceBlocked() const
	{
		return m_lowerPriceBlocked;
	}

}