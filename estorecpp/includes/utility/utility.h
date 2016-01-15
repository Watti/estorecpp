#ifndef ES_UTILITY_H
#define ES_UTILITY_H

#include "QString"
#include "QWidget"


static const QString DEFAULT_DB_NUMERICAL_TO_DISPLAY = "0";

namespace ES
{
	class Utility
	{
	public:
		Utility();
		~Utility();
		static bool verifyUsingMessageBox(QWidget *parent, QString title, QString message);
	private:

	};
}
#endif