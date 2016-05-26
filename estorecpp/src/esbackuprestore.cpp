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
	QString Cmd = QString("C:\\wamp64\\bin\\mysql\\mysql5.7.9\\bin\\mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish").arg("root", "123");
	QString Path = QString("%1").arg(m_backupPath+"\\goldfishdump.sql");
 	QProcess *poc = new QProcess(this);

	poc->setStandardOutputFile(Path);
	poc->start(Cmd);
	poc->waitForFinished(-1);
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
