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
				  QSqlQuery q5("ALTER TABLE bill AUTO_INCREMENT = 1");
				  QSqlQuery q6("ALTER TABLE sale AUTO_INCREMENT = 1");
				  QSqlQuery q7("ALTER TABLE return_item AUTO_INCREMENT = 1");
				  QSqlQuery q8("ALTER TABLE return_bill AUTO_INCREMENT = 1");
				  this->close();
			  }
		break;
	}
	default:
		break;
	}
}
