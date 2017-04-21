#include "essystemsettings.h"
#include "QSqlQuery"
#include "utility\utility.h"
#include <wingdi.h>
#include "easylogging++.h"
#include "utility\session.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
				  QSqlQuery q("DELETE FROM stock_audit");
				  QSqlQuery q10("ALTER TABLE bill AUTO_INCREMENT = 1");
				  QSqlQuery q11("ALTER TABLE sale AUTO_INCREMENT = 1");
				  QSqlQuery q12("ALTER TABLE return_item AUTO_INCREMENT = 1");
				  QSqlQuery q13("ALTER TABLE return_bill AUTO_INCREMENT = 1");
				  QSqlQuery q14("ALTER TABLE cash AUTO_INCREMENT = 1");
				  QSqlQuery q15("ALTER TABLE cheque AUTO_INCREMENT = 1");
				  QSqlQuery q16("ALTER TABLE credit AUTO_INCREMENT = 1");
				  QSqlQuery q17("ALTER TABLE card AUTO_INCREMENT = 1");
				  QSqlQuery q18("ALTER TABLE payment AUTO_INCREMENT = 1");
				  QSqlQuery q19("ALTER TABLE stock_audit AUTO_INCREMENT = 1");

				  int userId = ES::Session::getInstance()->getUser()->getId();
				  QString userIdStr;
				  userIdStr.setNum(userId);
				  QString auditQryStr("INSERT INTO backup_reset_audit (userId, action) VALUES('");
				  auditQryStr.append(userIdStr).append("', 'Reset')");
				  QSqlQuery qAudit(auditQryStr);
				  this->close();
				  LOG(INFO) << "Database has been reset by = " << ES::Session::getInstance()->getUser()->getName().toLatin1().toStdString();
			  }
		break;
	}
	default:
		break;
	}
}
