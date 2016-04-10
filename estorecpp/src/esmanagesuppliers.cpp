#include "esmanagesuppliers.h"
#include "esmanagesupplieritem.h"
#include "utility/esdbconnection.h"
#include "utility/esmainwindowholder.h"
#include <QMessageBox>
#include <QSignalMapper>

ESManageSuppliers::ESManageSuppliers(QWidget *parent /*= 0*/)
: QWidget(parent)
{
	ui.setupUi(this);
	ui.tableArea->show();
	ui.detailsArea->hide();
	ui.addSupplierItemBtn->hide();

	m_updateButtonSignalMapper = new QSignalMapper(this);
	m_removeButtonSignalMapper = new QSignalMapper(this);

	QObject::connect(m_updateButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotUpdate(QString)));
	QObject::connect(m_removeButtonSignalMapper, SIGNAL(mapped(QString)), this, SLOT(slotRemove(QString)));
	QObject::connect(ui.addNewSupplier, SIGNAL(clicked()), this, SLOT(slotShowAddSupplierView()));
	QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slotAddSupplier()));
	QObject::connect(ui.addSupplierItemBtn, SIGNAL(clicked()), this, SLOT(slotShowAddSupplierItemView()));

	QStringList headerLabels;
	headerLabels.append("Supplier Code");
	headerLabels.append("Supplier Name");
	headerLabels.append("Phone");
	headerLabels.append("Fax");
	headerLabels.append("E-mail");
	headerLabels.append("Address");
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

		QString supplierId = q.value("supplier_id").toString();

		QWidget* base = new QWidget(ui.tableWidget);
		QPushButton* updateBtn = new QPushButton("Update", base);
		updateBtn->setMaximumWidth(100);
		QPushButton* removeBtn = new QPushButton("Remove", base);
		removeBtn->setMaximumWidth(100);

		m_updateButtonSignalMapper->setMapping(updateBtn, supplierId);
		QObject::connect(updateBtn, SIGNAL(clicked()), m_updateButtonSignalMapper, SLOT(map()));

		QObject::connect(removeBtn, SIGNAL(clicked()), m_removeButtonSignalMapper, SLOT(map()));
		m_removeButtonSignalMapper->setMapping(removeBtn, supplierId);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(updateBtn);
		layout->addWidget(removeBtn);
		layout->insertStretch(2);
		base->setLayout(layout);
		ui.tableWidget->setCellWidget(row, 6, base);
		base->show();
	}
}

void ESManageSuppliers::slotUpdate(QString id)
{
	QString supplierId = id;

	ui.tableArea->hide();
	ui.detailsArea->show();
	ui.pushButton->setText("  Update  ");
	ui.addSupplierItemBtn->show();
}

void ESManageSuppliers::slotRemove(QString id)
{
	QString str("UPDATE supplier SET deleted = 1 WHERE supplier_id = " + id);
	QSqlQuery q;
	if (q.exec(str))
	{
		while (ui.tableWidget->rowCount() > 0)
		{
			ui.tableWidget->removeRow(0);
		}
		
		slotSearch();
	}
}

void ESManageSuppliers::slotShowAddSupplierItemView()
{
	ESManageSupplierItem* manageSupplierItems = new ESManageSupplierItem(this);
	ES::MainWindowHolder::instance()->getMainWindow()->setCentralWidget(manageSupplierItems);
	manageSupplierItems->show();
}
