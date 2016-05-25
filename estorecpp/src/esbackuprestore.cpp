#include "esbackuprestore.h"
#include <QFileDialog> 

ESBackupRestore::ESBackupRestore(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	QObject::connect(ui.backupDirectoryButton, SIGNAL(clicked()), this, SLOT(slotOpenFileDialog()));

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

void ESBackupRestore::slotTest()
{

}

void ESBackupRestore::slotOpenFileDialog()
{
	QString backupDirectoryPath = QFileDialog::getOpenFileName(this, tr("Open Backup File"));
	ui.backupDirectoryPath->setText(backupDirectoryPath);


}
