#ifndef ESReminders_h__
#define ESReminders_h__

#include "ui_reminders.h"
#include <QtGui>

class ESReminders : public QWidget
{
	Q_OBJECT

public:
	ESReminders(QWidget *parent = 0);
	~ESReminders();

	Ui::RemindersWidget& getUI() { return ui; }

	public slots:
	void slotShowAddItem();
	void slotPaint(int index);
	void setCurrentIndex(int index);

private:
	Ui::RemindersWidget ui;
};

#endif // esreminders_h__
