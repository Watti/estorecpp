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
		QString uName = ui.usernameText->text();
		QString pWord = ui.passwordText->text();
		QString encryptedPWrod = QString(QCryptographicHash::hash((pWord.toStdString().c_str()), QCryptographicHash::Md5).toHex());
		QSqlQuery query("SELECT * FROM user WHERE username = '" + uName + "' AND password = '" + encryptedPWrod + "'");

		if (query.next())
		{
			QString displayName = query.value("display_name").toString();
			QString typeId = query.value("usertype_id").toString();
			QSqlQuery query1("SELECT * FROM usertype WHERE usertype_id = " + typeId);
			QString perms = "";
			while (query1.next())
			{
				QString userType = query1.value("usertype_name").toString();
				ES::User* user = new ES::User;
				user->setId(query.value("user_id").toInt());
				perms = query1.value("permission").toString();
				user->setName(displayName);
				if (QString::compare(userType, "DEV") == 0)
				{
					user->setType(ES::User::DEV);
				}
				else if (QString::compare(userType, "MANAGER") == 0)
				{
					user->setType(ES::User::MANAGER);
				}
				else if (QString::compare(userType, "STORE KEEPER") == 0)
				{
					user->setType(ES::User::STORE_KEEPER);
				}
				else if (QString::compare(userType, "CASHIER") == 0)
				{
					user->setType(ES::User::CASHIER);
				}
				else if (QString::compare(userType, "SENIOR MANAGER") == 0)
				{

					user->setType(ES::User::SENIOR_MANAGER);
				}
				else
				{
					assert(false);
				}
				ES::Session* session = ES::Session::getInstance();
				session->setUser(user);

				// for testing 
// 				QString perms = "Stock,Items,Billing,Orders,Customers,Reports,Cash Balance,System,Business, \
// 						Help, User, Manage Stock Items, Manage Items, Manage Item Categories, Return Items, \
// 						Orders, Manage Order Items, Manage Suppliers, Manage Customers, View History, \
// 						Add Bill, Current Bills, Configure, Show Status, General, Sales, Stocks, Settings, \
// 						Backup & Restore, Offers, Gift Vouchers, Profile, Logout, Manage Users";
				ES::MenuManager::instance()->reload(perms);

				this->hide();
			}
		}
		else
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Authentication failed"));
			mbox.exec();
		}
	}
}

void ESLoginWidget::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			slotLogin();
			break;
		}
		default:
			QWidget::keyPressEvent(event);
	}
}
