#include "esbackuprestore.h"
#include <QFileDialog> 
#include "..\includes\utility\session.h"
#include "..\includes\utility\esdbconnection.h"
#include <QMessageBox>
#include "QDebug"

ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent), m_backupCopy(false)
{
	ui.setupUi(this);

	QObject::connect(ui.backupDirectoryButton, SIGNAL(clicked()), this, SLOT(slotOpenBackupFileDialog())); 
	QObject::connect(ui.manualDirectoryPathButton, SIGNAL(clicked()), this, SLOT(slotOpenCopyBackupFileDialog()));
	QObject::connect(ui.openRestoreFileDirectoryBtn, SIGNAL(clicked()), this, SLOT(slotOpenRestoreFileDialog()));
	QObject::connect(ui.manualRestoreBtn, SIGNAL(clicked()), this, SLOT(slotRestore()));
	QObject::connect(ui.manualBackupBtn, SIGNAL(clicked()), this, SLOT(slotBackupDatabaseManually()));
	QObject::connect(ui.copyToDirectoryCheckbox, SIGNAL(clicked()), this, SLOT(slotEnableCopyDirectory()));
	QObject::connect(ui.copyToDirectoryCheckbox, SIGNAL(clicked()), this, SLOT(slotEnableCopyDirectory()));
	QObject::connect(ui.fromManualLocationRadio, SIGNAL(toggled(bool)), this, SLOT(slotEnableManualRestore()));
	QObject::connect(ui.standardLocationRadio, SIGNAL(toggled(bool)), this, SLOT(slotEnableStandardRestore()));

	ui.backupDirectoryPath->setText(ES::Session::getInstance()->getBackupPath());
	m_backupPath = ES::Session::getInstance()->getBackupPath();
	if (!ui.copyToDirectoryCheckbox->isChecked())
	{
		ui.manualDirectoryPathLabel->setEnabled(false);
		ui.manualDirectoryPathText->setEnabled(false);
		ui.manualDirectoryPathButton->setEnabled(false);
	}
	ui.restoreFileText->setEnabled(false);
	ui.openRestoreFileDirectoryBtn->setEnabled(false);
	ui.tableWidget->setEnabled(false);
}

ESBackupRestore::~ESBackupRestore()
{
	 
}

void ESBackupRestore::slotBackupDatabaseManually()
{
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESBackupRestore"));
		mbox.exec();
	}
	QString fileName("goldfishdump.sql");
	QString cmd = QString("mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish").arg("root", "123");
	QString bckpPath = m_backupPath + "/" + fileName;
 	QProcess *poc = new QProcess(this);

	poc->setStandardOutputFile(bckpPath);
	poc->start(cmd);
	poc->waitForFinished(-1);
	if (m_backupCopy)
	{
		QString bkpCopyPath = m_copyPath + "/" + fileName;
		QFile::copy(bckpPath, bkpCopyPath);
	}
}

void ESBackupRestore::slotOpenBackupFileDialog()
{
	QString backupDirectoryPath = QFileDialog::getExistingDirectory(this, tr("Open Backup Directory"));
	ui.backupDirectoryPath->setText(backupDirectoryPath);
	m_backupPath = backupDirectoryPath;
	
}

QString ESBackupRestore::getBackupPath() const
{
	return m_backupPath;
}

void ESBackupRestore::slotEnableCopyDirectory()
{
	if (ui.copyToDirectoryCheckbox->isChecked())
	{
		m_backupCopy = true;
		ui.manualDirectoryPathLabel->setEnabled(true);
		ui.manualDirectoryPathText->setEnabled(true);
		ui.manualDirectoryPathButton->setEnabled(true);
	}
	else
	{
		m_backupCopy = false;
		ui.manualDirectoryPathLabel->setEnabled(false);
		ui.manualDirectoryPathText->setEnabled(false);
		ui.manualDirectoryPathButton->setEnabled(false);
	}
}

void ESBackupRestore::slotOpenCopyBackupFileDialog()
{
	QString copyDirectoryPath = QFileDialog::getExistingDirectory(this, tr("Open Copy Directory"));
	ui.manualDirectoryPathText->setText(copyDirectoryPath);
	m_copyPath = copyDirectoryPath;
}

void ESBackupRestore::slotOpenRestoreFileDialog()
{
	QString filter = "File Description (*.sql)";
	QString restorePath = QFileDialog::getOpenFileName(this, "Select a file...", m_backupPath, filter);
	ui.restoreFileText->setText(restorePath);
	m_manualRestorePath = restorePath;
}

void ESBackupRestore::slotEnableManualRestore()
{
	if (ui.fromManualLocationRadio->isChecked())
	{
		ui.restoreFileText->setEnabled(true);
		ui.openRestoreFileDirectoryBtn->setEnabled(true);
		ui.fromManualLocationRadio->setChecked(true);
		ui.standardLocationRadio->setChecked(false);
	}
	else
	{
		ui.restoreFileText->setEnabled(false);
		ui.openRestoreFileDirectoryBtn->setEnabled(false);
	}
}

void ESBackupRestore::slotRestore()
{
	if (ui.fromManualLocationRadio->isEnabled())
	{

	}
	else if (ui.standardLocationRadio->isEnabled())
	{

	}
	//QString fileName("goldfishdump.sql");
	//QString cmd = QString("-u%1 -p%2 goldfish").arg("root", "123");
	//QString bckpPath = m_backupPath + "/" + fileName;
	//cmd.append(m_manualRestorePath);
	QProcess *poc = new QProcess(this);

	QString cmd = QString("mysql.exe --log-error restore.log -u%1 -p%2 goldfish").arg("root", "123");
	QString Path = QString("%1").arg(m_manualRestorePath);
	poc->setStandardInputFile(Path);

// 	QStringList args;
// 	args << "--user=root" << "--password=123" << "--host=localhost";

 	poc->start(cmd);
	poc->waitForFinished(-1);
	qDebug() << poc->errorString();
}

void ESBackupRestore::slotEnableStandardRestore()
{

	if (ui.standardLocationRadio->isChecked())
	{
		ui.tableWidget->setEnabled(true);
		ui.standardLocationRadio->setChecked(true);
		ui.fromManualLocationRadio->setChecked(false);
	}
	else
	{
		ui.tableWidget->setEnabled(false);
	}
}
