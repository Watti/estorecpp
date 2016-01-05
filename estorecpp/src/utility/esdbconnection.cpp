#include "utility/esdbconnection.h"

namespace ES
{
	DbConnection* DbConnection::gObject = NULL;

	DbConnection::DbConnection()
	{
		m_isOpen = false;
	}

	DbConnection::~DbConnection()
	{

	}

	DbConnection* DbConnection::instance()
	{
		if (!gObject)
		{
			gObject = new DbConnection;
		}
		return gObject;
	}

	bool DbConnection::open()
	{
		if (!m_isOpen)
		{
			m_db = QSqlDatabase::addDatabase("QMYSQL");
			m_db.setHostName("localhost");
			m_db.setDatabaseName("goldfish");
			m_db.setUserName("root");
			m_db.setPassword("");

			if (!m_db.open())
			{
				return false;
			}

			m_isOpen = true;
		}
		
		return true;
	}

	void DbConnection::close()
	{
		if (m_isOpen)
		{
			m_db.close();
			m_isOpen = false;
		}
	}

}