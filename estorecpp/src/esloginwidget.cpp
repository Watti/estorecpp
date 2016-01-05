#include "esloginwidget.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include "QCryptographicHash.h"
#include "esmainwindow.h"
#include "esmanagestockitems.h"
#include "utility/esmenumanager.h"

ESLoginWidget::ESLoginWidget(QWidget* parent)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(slotLogin()));
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

		if (query.first())
		{
			this->hide();
			// for testing 
			QString perms = "Stock,Items,Billing,Orders,Reports,Help, \
							Manage Stock Items, Manage Items, Manage Item Categories, \
							Manage Item Prices, Add Bill, Current Bills";
			ES::MenuManager::instance()->reload(perms);
		}
	}
}