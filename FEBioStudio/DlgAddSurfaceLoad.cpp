#include "stdafx.h"
#include "DlgAddSurfaceLoad.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <PreViewLib/FEModel.h>
#include <PreViewLib/FEProject.h>
#include <FEMLib/FEMKernel.h>
#include "CIntInput.h"

class Ui::CDlgAddSurfaceLoad
{
public:
	QLineEdit* name;
	QComboBox* step;
	QListWidget* type;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit; name->setPlaceholderText("(Leave blank for default)");
		step = new QComboBox;

		type = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(type);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));

		parent->setLayout(mainLayout);
	}

	void addItem(const std::string& name, int nid)
	{
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(name));
		item->setData(Qt::UserRole, nid);
		type->addItem(item);
	}
};

CDlgAddSurfaceLoad::CDlgAddSurfaceLoad(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddSurfaceLoad)
{
	setWindowTitle("Add Surface Load");

	ui->setupUi(this);

	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the loads
	vector<FEClassFactory*> l = FEMKernel::FindAllClasses(prj.GetModule(), FE_SURFACE_LOAD);
	for (int i=0; i<(int)l.size(); ++i)
	{
		FEClassFactory* fac = l[i];
		ui->addItem(fac->GetTypeStr(), fac->GetClassID());
	}
}

void CDlgAddSurfaceLoad::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	// get the variable
	QListWidgetItem* item = ui->type->currentItem();
	if (item)
	{
		m_ntype = item->data(Qt::UserRole).toInt();
	}
	else
	{
		QMessageBox::critical(this, windowTitle(), "Please select a load type.");
		return;
	}

	QDialog::accept();
}
