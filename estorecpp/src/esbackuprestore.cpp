#include "esbackuprestore.h"
#include <QFileDialog> 
#include "..\includes\utility\session.h"
#include "..\includes\utility\esdbconnection.h"
#include <QMessageBox>
#include "QDebug"
#include <wingdi.h>
#include "easylogging++.h"
#include "QDateTime"
#include <synchapi.h>

ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent), m_backupCopy(false) , m_backupFileName("DBBackupFile.sql")
{
	ui.setupUi(this); 

	QObject::connect(ui.backupDirectoryButton, SIGNAL(clicked()), this, SLOT(slotOpenBackupFileDialog()));
	QObject::connect(ui.applyBackupSchedule, SIGNAL(clicked()), this, SLOT(slotUpdateBackupSchedule()));
	QObject::connect(ui.manualDirectoryPathButton, SIGNAL(clicked()), this, SLOT(slotOpenCopyBackupFileDialog()));
	QObject::connect(ui.openRestoreFileDirectoryBtn, SIGNAL(clicked()), this, SLOT(slotOpenRestoreFileDialog()));
	QObject::connect(ui.manualRestoreBtn, SIGNAL(clicked()), this, SLOT(slotRestore()));
	QObject::connect(ui.manualBackupBtn, SIGNAL(clicked()), this, SLOT(slotBackupDatabaseManually()));
	QObject::connect(ui.copyToDirectoryCheckbox, SIGNAL(clicked()), this, SLOT(slotEnableCopyDirectory()));
	QObject::connect(ui.copyToDirectoryCheckbox, SIGNAL(clicked()), this, SLOT(slotEnableCopyDirectory()));
	QObject::connect(ui.fromManualLocationRadio, SIGNAL(toggled(bool)), this, SLOT(slotEnableManualRestore()));
	QObject::connect(ui.standardLocationRadio, SIGNAL(toggled(bool)), this, SLOT(slotEnableStandardRestore()));
	QObject::connect(ui.backupTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBackupTypeChanged()));

	ui.backupTypeCombo->addItem("DAILY", DAILY);
	ui.backupTypeCombo->addItem("WEEKLY", WEEKLY);
	ui.backupTypeCombo->addItem("MONTHLY", MONTHLY);

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

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : ESBackupRestore"));
		mbox.exec();
		LOG(ERROR) << "Database connection failure in ESBackupRestore()";
	}
	QString q("SELECT * FROM backup_status");
	QSqlQuery query(q);
	if (query.next())
	{
		int type = query.value("backup_type").toInt();
		ui.backupSpinBox->setValue(query.value("repeating_value").toInt());
		ui.backupTypeCombo->setCurrentIndex(type-1);
	}
}

ESBackupRestore::~ESBackupRestore()
{
	 
}

void ESBackupRestore::slotBackupDatabaseManually()
{
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

void ESBackupRestore::slotUpdateBackupSchedule()
{
	int repeatValue = ui.backupSpinBox->value();
	int status = ui.backupTypeCombo->currentData().toInt();
	AutoBackupType bkpType = AutoBackupType(status);
	QString q("UPDATE backup_status SET backup_type=" + QString::number(bkpType) + ", 	repeating_value = " + QString::number(repeatValue) + ", updated_user=" + QString::number(ES::Session::getInstance()->getUser()->getId()));
	QSqlQuery query;
	if (!query.exec(q))
	{
		LOG(ERROR) << "ESBackupRestore::slotUpdateBackupSchedule : " << q.toLatin1().data();
	}
}

void ESBackupRestore::slotBackupTypeChanged()
{
	switch (ui.backupTypeCombo->currentData().toInt())
	{
	case DAILY:
		ui.backupSpinBox->setRange(1, 7);
		break;
	case WEEKLY:
		ui.backupSpinBox->setRange(1, 4);
		break;
	case MONTHLY:
		ui.backupSpinBox->setRange(1, 12);
		break;
	}
}

void BackupThread::run()
{
	while (true){
		if (!ES::DbConnection::instance()->open())
		{
			QMessageBox mbox;
			mbox.setIcon(QMessageBox::Critical);
			mbox.setText(QString("Cannot connect to the database : BackupThread::run"));
			mbox.exec();
			LOG(ERROR) << "Database connection failure in BackupThread::run()";
		}

		QString q("SELECT * FROM backup_status");
		QSqlQuery query(q);
		if (query.next())
		{
			int type = query.value("backup_type").toInt();
			int repeat = query.value("repeating_value").toInt();
			QDate lastBakupDate = query.value("last_backup_date").toDate();
			QDate currentDate(QDate::currentDate());

			QString bkpPath = ES::Session::getInstance()->getBackupPath();
			QString timeStanmp = QDateTime::currentDateTime().toString("yyyy-MM-dd");
			QString bakupFileName = "DBBackupFile-" + timeStanmp + ".sql";
			switch (type)
			{
			case 1:
			{
					  int daysFromLastBakup = (int)lastBakupDate.daysTo(currentDate);
					  if (daysFromLastBakup >= repeat)
					  {

						  QString cmd = QString("mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish").arg("root", "123");
						  QString bckpPath = bkpPath + "\\" + bakupFileName;
						  QProcess *poc = new QProcess(this);

						  poc->setStandardOutputFile(bckpPath);
						  poc->start(cmd);
						  poc->waitForFinished(-1);

						  q = "UPDATE backup_status SET last_bakup_date ="+currentDate.toString();
						  QSqlQuery queryUpdate;
						  if (!queryUpdate.exec(q))
						  {
							  LOG(ERROR) << "ESBackupRestore::slotUpdateBackupSchedule : " << q.toLatin1().data();
						  }

// 						  qDebug() << poc->errorString();
// 						  qDebug() << bkpPath;
// 						  qDebug() << bakupFileName;
// 						  qDebug() << bckpPath;
					  }
			}
				break;
			case 2:
				break;
			case 3:
				break;
			default:
				break;
			}
		}


		Sleep(1000000);
	}


}
