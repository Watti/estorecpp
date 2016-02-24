#include "esmanageusers.h"

ESManageUsers::ESManageUsers(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.addButton, SIGNAL(clicked()), this, SLOT(slotAdd()));
	QObject::connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(slotClear()));
	QObject::connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(slotUpdate()));
	QObject::connect(ui.roleComboSearch, SIGNAL(activated(QString)), this, SLOT(slotRoleSearch()));


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

}
