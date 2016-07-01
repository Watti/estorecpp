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
			m_db.setHostName("localhost");//192.168.1.6
			m_db.setDatabaseName("goldfish");
			m_db.setUserName("root");//prog
			m_db.setPassword("123");//progex@2016

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