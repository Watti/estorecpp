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
	void backupDatabase();
	void slotOpenFileDialog();

private:
	Ui::BackupRestoreWidget ui;
	QString m_backupPath;
	QString getBackupPath() const;
};

#endif