#ifndef esauthentication_h__
#define esauthentication_h__

#include "ui_authentication.h"
#include <QtGui>
#include <QPrinter>
#include <KDReportsReport.h>
#include "QDialog"

class ESAuthentication : public QDialog
{
	Q_OBJECT

public:
	ESAuthentication(QDialog *parent = 0);
	~ESAuthentication();

	Ui::AuthenticationWidget& getUI() { return ui; };

	public slots:
	void slotAuthenticate();

protected:
	void keyPressEvent(QKeyEvent * event);

private:
	Ui::AuthenticationWidget ui;

};

#endif // esauthentication_h__
