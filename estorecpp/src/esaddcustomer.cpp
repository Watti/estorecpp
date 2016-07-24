#include "esaddcustomer.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QSqlError>
#include "QString"

ESAddCustomer::ESAddCustomer(QWidget *parent /*= 0*/) : QWidget(parent), m_update(false)
{
	ui.setupUi(this);

	ui.active->setChecked(true);
	QObject::connect(ui.button, SIGNAL(clicked()), this, SLOT(slotProcess()));

	if (m_update)
	{
		ui.button->setText(QString(" Update "));
	}
	else
	{
		ui.button->setText(QString(" Add "));
	}

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESAddCustomer"));
		mbox.exec();
	}
}

ESAddCustomer::~ESAddCustomer()
{

}

void ESAddCustomer::slotProcess()
{
	QString name = ui.nameText->text();
	QString phone = ui.phoneText->text();
	QString address = ui.addressText->text();
	QString comments = ui.commentsText->toPlainText();
	bool active = ui.active;

	if (name == nullptr || name.isEmpty())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Warning);
		mbox.setText(QString("Name cannot be Empty"));
		mbox.exec();
	}
	else
	{
		int deleted = 0;
		if (!active)
		{
			deleted = 0;
		}
		QString qStr = "";
		if (m_update)
		{
			qStr = "UPDATE customer SET name = '"+name+"', phone = '"+phone+"', address = '"+address+"', comments = '"+comments+"', deleted ='"+QString::number(deleted)+"' WHERE customer_id ='"+m_id+"'";
		}
		else
		{
			qStr = "INSERT INTO customer (name, phone, address, comments, deleted) VALUES ('" + name + "','" + phone + "', '" + address + "','" + comments + "','" + QString::number(deleted) + "')";
		}

		QSqlQuery query;
		if (query.exec(qStr))
		{
			this->close();
		}
		else
		{
			QString err = query.lastError().text();
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong:: customer information cannot be saved"));
			mbox.exec();
		}
	}
}

void ESAddCustomer::setUpdate(bool update)
{
	m_update = update;
}

void ESAddCustomer::setCustomerId(QString val)
{
	m_id = val;
}
