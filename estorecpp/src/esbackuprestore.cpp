#include "esbackuprestore.h"
#include <QFileDialog> 
#include "..\includes\utility\session.h"
#include "..\includes\utility\esdbconnection.h"
#include <QMessageBox>

ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.backupDirectoryButton, SIGNAL(clicked()), this, SLOT(slotOpenFileDialog()));
	QObject::connect(ui.manualBackupBtn, SIGNAL(clicked()), this, SLOT(backupDatabase()));
	ui.backupDirectoryPath->setText(ES::Session::getInstance()->getDumpPath());
	m_backupPath = ES::Session::getInstance()->getDumpPath();
	if (!ui.copyToDirectoryCheckbox->isChecked())
	{
		ui.manualDirectoryPathLabel->setEnabled(false);
		ui.manualDirectoryPathText->setEnabled(false);
		ui.manualDirectoryPathButton->setEnabled(false);
	}
}

ESBackupRestore::~ESBackupRestore()
{

}

void ESBackupRestore::backupDatabase()
{
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESBackupRestore"));
		mbox.exec();
	}
	QString Cmd = QString("C:\wamp\bin\mysql\mysql5.6.17\bin\mysqldump --add-drop-table -u%1 -p%2 goldfish").arg("root", "");
	QString Path = QString("%1").arg(m_backupPath + "\\goldfishdump.sql");
	QProcess *poc = new QProcess;
	poc->setStandardOutputFile(Path);
	poc->start(Cmd);
	poc->waitForFinished();
	qDebug() << poc->errorString();

	//restore
	/*
	 QString Cmd = QString("mysql.exe -u%1 -p%2 test").arg("UsrName","UsrPsd");
	 QString Path = QString("%1").arg("d://backup.Sql");
	 QProcess *poc=new QProcess;
	 poc->setStandardInputFile(Path);
	 poc->start(Cmd);
	*/
}

void ESBackupRestore::slotOpenFileDialog()
{
	QString backupDirectoryPath = QFileDialog::getOpenFileName(this, tr("Open Backup File"));
	ui.backupDirectoryPath->setText(backupDirectoryPath);
	m_backupPath = backupDirectoryPath;
	
}

QString ESBackupRestore::getBackupPath() const
{
	return m_backupPath;
}
