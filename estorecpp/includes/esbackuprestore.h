#ifndef ES_BACKUP_RESTORE_H
#define ES_BACKUP_RESTORE_H

#include "ui_backuprestore.h"
#include <QtGui>
#include "QThread"

class BackupThread : public QThread
{
	Q_OBJECT

protected:
	void run();
};

class ESBackupRestore : public QWidget
{
	Q_OBJECT

public:
	enum AutoBackupType
	{
		DAILY = 1, 
		WEEKLY,
		MONTHLY
	};
	ESBackupRestore(QWidget *parent = 0);
	~ESBackupRestore();
	public slots:
	void slotBackupDatabaseManually();
	void slotOpenBackupFileDialog();
	void slotOpenCopyBackupFileDialog();
	void slotOpenRestoreFileDialog();
	void slotEnableCopyDirectory();
	void slotEnableManualRestore();
	void slotRestore();
	void slotEnableStandardRestore();
	void slotUpdateBackupSchedule();
	void slotBackupTypeChanged();

private:
	Ui::BackupRestoreWidget ui;
	QString m_backupPath;
	QString m_copyPath;
	bool m_backupCopy;
	QString m_manualRestorePath;
	QString m_backupFileName;
	QString getBackupPath() const;

};

#endif