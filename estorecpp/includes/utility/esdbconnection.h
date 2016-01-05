#ifndef ES_DBCONNECTION_H
#define ES_DBCONNECTION_H
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>

namespace ES
{

	class DbConnection
	{
	public:
		~DbConnection();

		static DbConnection* instance();

		bool open();

		void close();

	private:
		DbConnection();

		QSqlDatabase m_db;
		bool m_isOpen;
		static DbConnection* gObject;

	};

}

#endif