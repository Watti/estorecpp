#include "esaddcustomer.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>
#include <QSqlError>

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
		mbox.setText(QString("Cannot connect to the database : AddItemCategory"));
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
		QSqlQuery query;
		if (m_update)
		{
			query.prepare("UPDATE customer SET name = '?', phone = '?', address = '?', comments = '?', deleted = ? WHERE customer_id = ?");
			query.addBindValue(name);
			query.addBindValue(phone);
			query.addBindValue(address);
			query.addBindValue(comments);
			query.addBindValue(!active);
			query.addBindValue(m_id);
		}
		else
		{
			query.prepare("INSERT INTO customer (name, phone, address, comments, deleted) VALUES (?, ?, ?, ?, ?)");
			query.addBindValue(name);
			query.addBindValue(phone);
			query.addBindValue(address);
			query.addBindValue(comments);
			query.addBindValue(!active);
		}
		
		if (query.exec())
		{
			this->close();
		}
		else
		{
			QString err = query.lastError().text();
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Something goes wrong:: Category cannot be saved"));
			mbox.exec();
		}
	}
}

void ESAddCustomer::setUpdate(bool update)
{
	m_update = update;
}
