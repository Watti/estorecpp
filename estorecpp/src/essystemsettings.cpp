#include "essystemsettings.h"
#include "QSqlQuery"
#include "utility\utility.h"

ESSystemSettings::ESSystemSettings(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.resetButton, SIGNAL(clicked()), this, SLOT(resetDatabase()));
}

ESSystemSettings::~ESSystemSettings()
{

}

void ESSystemSettings::resetDatabase()
{
	int resetTypeIndex = ui.comboBox->currentIndex();

	switch (resetTypeIndex)
	{
	case 0:
	{
			  if (ES::Utility::verifyUsingMessageBox(this, "Progex", "Do you really want to reset the database this?"))
			  {
				  QSqlQuery q1("DELETE FROM bill");
				  QSqlQuery q2("DELETE FROM sale");
				  QSqlQuery q3("DELETE FROM return_item");
				  QSqlQuery q4("DELETE FROM return_bill");
				  QSqlQuery q5("DELETE FROM cash");
				  QSqlQuery q6("DELETE FROM cheque");
				  QSqlQuery q7("DELETE FROM credit");
				  QSqlQuery q8("DELETE FROM payment");
				  QSqlQuery q9("DELETE FROM card");
				  QSqlQuery q10("ALTER TABLE bill AUTO_INCREMENT = 1");
				  QSqlQuery q11("ALTER TABLE sale AUTO_INCREMENT = 1");
				  QSqlQuery q12("ALTER TABLE return_item AUTO_INCREMENT = 1");
				  QSqlQuery q13("ALTER TABLE return_bill AUTO_INCREMENT = 1");
				  QSqlQuery q14("ALTER TABLE cash AUTO_INCREMENT = 1");
				  QSqlQuery q15("ALTER TABLE cheque AUTO_INCREMENT = 1");
				  QSqlQuery q16("ALTER TABLE credit AUTO_INCREMENT = 1");
				  QSqlQuery q17("ALTER TABLE card AUTO_INCREMENT = 1");
				  QSqlQuery q18("ALTER TABLE payment AUTO_INCREMENT = 1");
				  this->close();
			  }
		break;
	}
	default:
		break;
	}
}
