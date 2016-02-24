#ifndef ESMANAGE_USERS_H
#define ESMANAGE_USERS_H

#include "ui_manageusers.h"

class ESManageUsers : public QWidget
{
	Q_OBJECT

public:
	ESManageUsers(QWidget *parent = 0);
	~ESManageUsers();

	Ui::ManageUsers& getUI();

public slots:
	void slotClear();
	void slotUpdate();
	void slotAdd();
	void slotRoleSearch();

private:
	Ui::ManageUsers ui;

};

#endif