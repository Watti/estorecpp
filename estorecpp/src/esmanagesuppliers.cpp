#include "esmanagesuppliers.h"
#include "utility/esdbconnection.h"
#include <QMessageBox>

ESManageSuppliers::ESManageSuppliers(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);

	ui.tableArea->show();
	ui.detailsArea->hide();

	QObject::connect(ui.addNewSupplier, SIGNAL(clicked()), this, SLOT(slotShowAddSupplierView()));
	QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slotAddSupplier()));

	QStringList headerLabels;
	headerLabels.append("Supplier Code");
	headerLabels.append("Supplier Name");
	headerLabels.append("Phone");
	headerLabels.append("Fax");
	headerLabels.append("E-mail");
	headerLabels.append("Actions");

	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	if (!ES::DbConnection::instance()->open())
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("Database Connection Error : Suppliers"));
		mbox.exec();
	}
	else
	{
		slotSearch();
	}
}

ESManageSuppliers::~ESManageSuppliers()
{

}

void ESManageSuppliers::slotAddSupplier()
{
	QString query;
	query.append("INSERT INTO supplier (supplier_code, supplier_name, phone, fax, email, address) VALUES ('");
	query.append(ui.code->text());
	query.append("', '");
	query.append(ui.name->text());
	query.append("', ");
	query.append(ui.phone->text());
	query.append(", ");
	query.append(ui.fax->text());
	query.append(", '");
	query.append(ui.email->text());
	query.append("', '");
	query.append(ui.address->toPlainText());
	query.append("')");

	QSqlQuery q;
	if (!q.exec(query))
	{
		QMessageBox mbox;
		mbox.setIcon(QMessageBox::Critical);
		mbox.setText(QString("insertion error :: cannot reduce this quantity from the main stock order"));
		mbox.exec();
	}

	ui.tableArea->show();
	ui.detailsArea->hide();
}

void ESManageSuppliers::slotShowAddSupplierView()
{
	ui.tableArea->hide();
	ui.detailsArea->show();
	ui.pushButton->setText("  Add  ");
}

void ESManageSuppliers::slotShowUpdateSupplierView()
{
	ui.tableArea->hide();
	ui.detailsArea->show();
	ui.pushButton->setText("  Update  ");
}

void ESManageSuppliers::slotSearch()
{
	QSqlQuery q("SELECT * FROM supplier WHERE deleted = 0");

	while (ui.tableWidget->rowCount() > 0)
	{
		ui.tableWidget->removeRow(0);
	}

	while (q.next())
	{
		int row = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(row);

		ui.tableWidget->setItem(row, 0, new QTableWidgetItem(q.value("supplier_code").toString()));
		ui.tableWidget->setItem(row, 1, new QTableWidgetItem(q.value("supplier_name").toString()));
		ui.tableWidget->setItem(row, 2, new QTableWidgetItem(q.value("phone").toString()));
		ui.tableWidget->setItem(row, 3, new QTableWidgetItem(q.value("fax").toString()));
		ui.tableWidget->setItem(row, 4, new QTableWidgetItem(q.value("email").toString()));
		ui.tableWidget->setItem(row, 5, new QTableWidgetItem(q.value("address").toString()));
	}
}
