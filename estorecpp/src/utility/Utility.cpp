
#include "utility/utility.h"
#include "QMessageBox"

namespace ES
{
	Utility::Utility()
	{

	}

	Utility::~Utility()
	{

	}

	bool Utility::verifyUsingMessageBox(QWidget *parent, QString title, QString message)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(parent, title, message,
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			return true;
		}
		else
		{
			return false;
		}
	}

}