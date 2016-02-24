#ifndef ESMANAGE_USERS_H
#define ESMANAGE_USERS_H

#include "ui_manageusers.h"
#include <QListWidgetItem>

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
	void slotUserSelected(QListWidgetItem* item);

private:
	Ui::ManageUsers ui;

};

#endif