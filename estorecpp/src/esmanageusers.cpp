#include "esmanageusers.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>

ESManageUsers::ESManageUsers(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAdd()));
	QObject::connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(slotClear()));
	QObject::connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(slotUpdate()));
	QObject::connect(ui.roleComboSearch, SIGNAL(activated(QString)), this, SLOT(slotRoleSearch()));
	QObject::connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotUserSelected(QListWidgetItem*)));

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESManageUsers"));
		mbox.exec();
	}
	else
	{
		ui.roleComboSearch->addItem("ALL", 0);
		QSqlQuery userRoleQuery("SELECT * FROM usertype");
		while (userRoleQuery.next())
		{
			int usertype_id = userRoleQuery.value(0).toInt();
			QString usertype_name = userRoleQuery.value(1).toString();

			if (usertype_name == "DEV") continue;

			ui.roleCombo->addItem(usertype_name, usertype_id);
			ui.roleComboSearch->addItem(usertype_name, usertype_id);
		}

		slotRoleSearch();
	}
}

ESManageUsers::~ESManageUsers()
{

}

Ui::ManageUsers& ESManageUsers::getUI()
{
	return ui;
}

void ESManageUsers::slotClear()
{

}

void ESManageUsers::slotUpdate()
{

}

void ESManageUsers::slotAdd()
{

}

void ESManageUsers::slotRoleSearch()
{
	ui.listWidget->clear();

	int role = ui.roleComboSearch->currentData().toInt();

	QSqlQuery userQuery("SELECT * FROM user");
	while (userQuery.next())
	{
		if (role > 0)
		{
			int userrole = userQuery.value(3).toInt();
			if (role != userrole) continue;
		}

		QListWidgetItem* item = new QListWidgetItem(userQuery.value(1).toString());
		item->setData(Qt::UserRole, userQuery.value(0).toInt());
		ui.listWidget->addItem(item);
	}
}

void ESManageUsers::slotUserSelected(QListWidgetItem* item)
{
	if (!item) return;

	QString q = "SELECT * FROM user WHERE user_id = " + (item->data(Qt::UserRole)).toString();
	QSqlQuery userQuery(q);
	if (userQuery.first())
	{
		ui.nameText->setText(userQuery.value(5).toString());
		ui.usernameText->setText(userQuery.value(1).toString());
		//ui.passwordText->setText(userQuery.value(2).toString());

		QString q2 = "SELECT * FROM usertype WHERE usertype_id = " + userQuery.value(3).toString();
		QSqlQuery userRoleQuery(q2);
		if (userRoleQuery.first())
		{
			ui.roleCombo->setCurrentText(userRoleQuery.value(1).toString());
		}
	}
}