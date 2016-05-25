#include "esbackuprestore.h"
#include <QFileDialog> 

ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.backupDirectoryButton, SIGNAL(clicked()), this, SLOT(slotOpenFileDialog()));
}

ESBackupRestore::~ESBackupRestore()
{

}

void ESBackupRestore::slotTest()
{

}

void ESBackupRestore::slotOpenFileDialog()
{
	QString backupDirectoryPath = QFileDialog::getOpenFileName(this, tr("Open Backup File"));
	ui.backupDirectoryPath->setText(backupDirectoryPath);
}
