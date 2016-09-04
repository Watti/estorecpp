#include "esauthentication.h"
#include "QSqlQuery"

ESAuthentication::ESAuthentication(bool& success, QDialog *parent /*= 0*/) 
: QDialog(parent), m_success(success)
{
	ui.setupUi(this);

	QObject::connect(ui.cancel, SIGNAL(clicked()), this, SLOT(close()));
	QObject::connect(ui.ok, SIGNAL(clicked()), this, SLOT(slotAuthenticate()));
}

ESAuthentication::~ESAuthentication()
{

}

void ESAuthentication::slotAuthenticate()
{
	QString uName = ui.username->text();
	QString pWord = ui.password->text();
	QString encryptedPWrod = QString(QCryptographicHash::hash((pWord.toStdString().c_str()), QCryptographicHash::Md5).toHex());

	QSqlQuery authQuery;
	authQuery.prepare("SELECT * FROM user WHERE username = ? AND password = ?");
	authQuery.addBindValue(uName);
	authQuery.addBindValue(encryptedPWrod);
	if (authQuery.exec())
	{
		if (authQuery.next())
		{
			QString usertype_id = authQuery.value("usertype_id").toString();
			QSqlQuery q("SELECT usertype_name FROM usertype WHERE usertype_id = " + usertype_id);
			if (q.next())
			{
				QString type = q.value("usertype_name").toString();
				if (type == "MANAGER" || type == "ASSITANT MANAGER")
				{
					m_success = true;
				}
			}
		}		
	}
	close();
}

void ESAuthentication::keyPressEvent(QKeyEvent * event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
	{
		slotAuthenticate();
		break;
	}
	default:
		QWidget::keyPressEvent(event);
	}
}
