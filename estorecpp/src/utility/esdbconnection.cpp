#include "utility/esdbconnection.h"
#include "utility/session.h"

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
			QString server = ES::Session::getInstance()->getServerIP();
			m_db.setHostName(server);//192.168.1.6
			m_db.setDatabaseName("goldfish");
			m_db.setUserName("prog");//prog
			m_db.setPassword("progex@2016");//progex@2016

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