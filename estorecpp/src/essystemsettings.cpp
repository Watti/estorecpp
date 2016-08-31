#include "essystemsettings.h"
#include "QSqlQuery"

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
		QSqlQuery q1("DELETE FROM bill");
		QSqlQuery q2("DELETE FROM sale");
		QSqlQuery q3("ALTER TABLE bill AUTO_INCREMENT = 1");
		QSqlQuery q4("ALTER TABLE sale AUTO_INCREMENT = 1");
		break;
	}
	default:
		break;
	}
}
