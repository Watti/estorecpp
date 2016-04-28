#include "esloginwidget.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QCryptographicHash.h"
#include "esmainwindow.h"
#include "esmanagestockitems.h"
#include "utility/esmenumanager.h"
#include "utility/session.h"
#include <assert.h>

ESLoginWidget::ESLoginWidget(QWidget* parent)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(slotLogin()));

	ui.usernameText->setFocus();
}

ESLoginWidget::~ESLoginWidget()
{

}

void ESLoginWidget::slotLogin()
{
	bool ok = ES::DbConnection::instance()->open();
	if (!ok)
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database"));
		mbox.exec();
	}
	else
	{
		QString uName = "indunil";//ui.usernameText->text();
		QString pWord = "123";/*ui.passwordText->text();*/
		QString encryptedPWrod = QString(QCryptographicHash::hash((pWord.toStdString().c_str()), QCryptographicHash::Md5).toHex());
		QSqlQuery query("SELECT * FROM user WHERE username = '" + uName + "' AND password = '" + encryptedPWrod + "'");

		if (query.next())
		{
			QString displayName = query.value(5).toString();
			QString typeId = query.value(3).toString();
			QSqlQuery query1("SELECT * FROM usertype WHERE usertype_id = " + typeId);
			while (query1.next())
			{
				QString userType = query1.value(1).toString();
				ES::User* user = new ES::User;
				user->setId(query.value(0).toInt());
				user->setName(displayName);
				if (userType.compare(QString("DEV")))
				{
					user->setType(ES::User::DEV);
				}
				else if (	userType.compare(QString("MANAGER")))
				{
					user->setType(ES::User::MANAGER);
				}
				else if (userType.compare(QString("STORE KEEPER")))
				{
					user->setType(ES::User::STORE_KEEPER);
				}
				else if (userType.compare(QString("CASHIER")))
				{
					user->setType(ES::User::CASHIER);
				}
				else
				{
					assert(false);
				}
				ES::Session* session = ES::Session::getInstance();
				session->setUser(user);

				// for testing 
				QString perms = "Stock,Items,Billing,Orders,Reports,Cash Balance,System, Help,User, \
						Manage Stock Items, Manage Items, Manage Item Categories, Return Items, \
						Orders, Manage Order Items, Manage Suppliers, Add Bill, Current Bills, \
						Configure, Show Status, General, Sales, Stocks, Settings, Backup & Restore, \
						Profile, Logout, Manage Users";
				ES::MenuManager::instance()->reload(perms);

				this->hide();
			}
		}
	}
}