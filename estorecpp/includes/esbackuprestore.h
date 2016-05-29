#ifndef ES_BACKUP_RESTORE_H
#define ES_BACKUP_RESTORE_H

#include "ui_backuprestore.h"
#include <QtGui>

class ESBackupRestore : public QWidget
{
	Q_OBJECT

public:
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

private:
	Ui::BackupRestoreWidget ui;
	QString m_backupPath;
	QString m_copyPath;
	bool m_backupCopy;
	QString m_manualRestorePath;
	QString getBackupPath() const;
};

#endif