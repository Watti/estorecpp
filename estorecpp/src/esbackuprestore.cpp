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
#include <QtGlobal>
#include "QFileInfo"
ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent), m_backupCopy(false), m_backupFileName("DBBackupFile.sql")
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
		ui.backupTypeCombo->setCurrentIndex(type - 1);
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
	//gunzip < [backupfile.sql.gz] | mysql -u [uname] -p[pass] [dbname]
	QString cmd = QString("mysql.exe -u%1 -p%2 goldfish").arg("prog", "progex@2016");
	QString Path = QString("%1").arg(m_manualRestorePath);
	poc->setStandardInputFile(Path);

	// 	QStringList args;
	// 	args << "--user=root" << "--password=123" << "--host=localhost";

	poc->start(cmd);
	bool success = poc->waitForFinished(-1);
	QProcess::ExitStatus status = poc->exitStatus();
	QProcess::ProcessError error = poc->error();
	if (status == QProcess::ExitStatus::NormalExit)
	{
		int userId = ES::Session::getInstance()->getUser()->getId();
		QString userIdStr;
		userIdStr.setNum(userId);
		QString auditQryStr("INSERT INTO backup_reset_audit (userId, action) VALUES('");
		auditQryStr.append(userIdStr).append("', 'Restore')");
		QSqlQuery qAudit(auditQryStr);
	}
	qDebug() << poc->errorString();
	this->close();
}

void ESBackupRestore::slotEnableStandardRestore()
{

	if (ui.standardLocationRadio->isChecked())
	{
		ui.tableWidget->setEnabled(true);
		ui.standardLocationRadio->setChecked(true);
		ui.fromManualLocationRadio->setChecked(false);

		QDir recoredDir(ES::Session::getInstance()->getBackupPath());
		QString filter = "File Description (*.sql)";
		QStringList allFiles = recoredDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Files);
		for (auto file : allFiles)
		{
			int row = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(row);
			QFileInfo InfoAboutCurrentFile(ES::Session::getInstance()->getBackupPath()+"/"+file);
			ui.tableWidget->setItem(row, 1, new QTableWidgetItem(file));
			ui.tableWidget->setItem(row, 0, new QTableWidgetItem(InfoAboutCurrentFile.created().toString("yyyy-MM-dd hh:mm:ss")));
		}
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
	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Cannot connect to the database : BackupThread::run"));
		mbox.exec();
		LOG(ERROR) << "Database connection failure in BackupThread::run()";
	}

	QString q("SELECT auto_backup FROM backup_status");
	QSqlQuery queryBackup(q);
	//if (queryBackup.next())
	{
		int autoBackup = 1;//queryBackup.value("auto_backup").toInt();
		if (autoBackup != 0)
		{
			//while (true)
			{
				//QString q("SELECT * FROM backup_status");
				//QSqlQuery query(q);
				//if (query.next())
				{
					int type = 1;//query.value("backup_type").toInt();
// 					int repeat = query.value("repeating_value").toInt();
// 					QDate lastBakupDate = query.value("last_backup_date").toDate();
// 					QDate currentDate(QDate::currentDate());

					switch (type)
					{
					case 1:
					{
							  //if (abs(currentDate.day() - lastBakupDate.day()) >= repeat)
							  {
								  QString bkpPath = ES::Session::getInstance()->getBackupPath();
								  QString timeStanmp = QDateTime::currentDateTime().toString("yyyy-MM-dd");
								  QString bakupFileName = "DBBackupFile-" + timeStanmp + ".sql";

								  QString cmd = QString("mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish bill card cash cheque credit payment petty_cash sale supplier purchase_order stock_purchase_order_item return_item supplier_item item item_category stock customer ").arg("root", "123");
								  QString bckpPath = bkpPath + "\\" + bakupFileName;
								  QProcess *poc = new QProcess(this);

								  poc->setStandardOutputFile(bckpPath);
								  poc->start(cmd);
								  poc->waitForFinished(-1);

// 								  QString qStr = "UPDATE backup_status SET last_backup_date = '" + currentDate.toString("yyyy-MM-dd") + "'";
// 								  QSqlQuery queryUpdate;
// 								  if (!queryUpdate.exec(qStr))
// 								  {
// 									  LOG(ERROR) << "ESBackupRestore::slotUpdateBackupSchedule : " << qStr.toLatin1().data();
// 								  }
							  }
					}
						break;
					case 2:
					{
// 							  int dl = lastBakupDate.month();
// 							  int dc = currentDate.month();
// 							  int t = abs(currentDate.month() - lastBakupDate.month());
// 							  int noOfWeeks = lastBakupDate.daysTo(currentDate) / 7;
// 							  if (noOfWeeks >= repeat)
							  {
								  QString bkpPath = ES::Session::getInstance()->getBackupPath();
								  QString timeStanmp = QDateTime::currentDateTime().toString("yyyy-MM-dd");
								  QString bakupFileName = "DBBackupFile-" + timeStanmp + ".sql";

								  QString cmd = QString("mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish").arg("root", "123");
								  QString bckpPath = bkpPath + "\\" + bakupFileName;
								  QProcess *poc = new QProcess(this);

								  poc->setStandardOutputFile(bckpPath);
								  poc->start(cmd);
								  poc->waitForFinished(-1);

// 								  QString qStr = "UPDATE backup_status SET last_backup_date = '" + currentDate.toString("yyyy-MM-dd") + "'";
// 								  QSqlQuery queryUpdate;
// 								  if (!queryUpdate.exec(qStr))
// 								  {
// 									  LOG(ERROR) << "ESBackupRestore::slotUpdateBackupSchedule : " << qStr.toLatin1().data();
// 								  }
							  }
					}
						break;
					case 3:
					{
							 // if (abs(currentDate.month() - lastBakupDate.month()) >= repeat)
							  {
								  QString bkpPath = ES::Session::getInstance()->getBackupPath();
								  QString timeStanmp = QDateTime::currentDateTime().toString("yyyy-MM-dd");
								  QString bakupFileName = "DBBackupFile-" + timeStanmp + ".sql";

								  QString cmd = QString("mysqldump.exe --log-error backup.log -u%1 -p%2 goldfish").arg("root", "123");
								  QString bckpPath = bkpPath + "\\" + bakupFileName;
								  QProcess *poc = new QProcess(this);

								  poc->setStandardOutputFile(bckpPath);
								  poc->start(cmd);
								  poc->waitForFinished(-1);

// 								  QString qStr = "UPDATE backup_status SET last_backup_date = '" + currentDate.toString("yyyy-MM-dd") + "'";
// 								  QSqlQuery queryUpdate;
// 								  if (!queryUpdate.exec(qStr))
// 								  {
// 									  LOG(ERROR) << "ESBackupRestore::slotUpdateBackupSchedule : " << qStr.toLatin1().data();
// 								  }
							  }
					}
						break;
					default:
						break;
					}
				}
				//Sleep(1000000);
			}
		}
	}


}
