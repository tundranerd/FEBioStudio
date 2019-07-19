#include "stdafx.h"
#include "DlgExportFEBio.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>

class Ui::CDlgExportFEBio
{
public:
	QComboBox*	combo;
	QCheckBox*  part;
	QCheckBox*	sel;
	QCheckBox*	comp;
	QCheckBox* pc[15];

public:
	void setupUi(QWidget* parent)
	{
		// format selector
		QLabel* l = new QLabel("format:");
		combo = new QComboBox;
		combo->addItem("febio_spec 1.2");
		combo->addItem("febio_spec 2.0");
		combo->addItem("febio_spec 2.5");
		combo->addItem("febio_spec 3.0");
		combo->setCurrentIndex(2);
		l->setBuddy(combo);

		QHBoxLayout* formatLayout = new QHBoxLayout;
		formatLayout->addWidget(l);
		formatLayout->addWidget(combo);
		formatLayout->addStretch();

		sel = new QCheckBox("export named selections (version 2.5)");
		sel->setChecked(true);

		part = new QCheckBox("export parts (version 2.5)");

		comp = new QCheckBox("Use plotfile compression");

		QVBoxLayout* topLayout = new QVBoxLayout;
		topLayout->addLayout(formatLayout);
		topLayout->addWidget(sel);
		topLayout->addWidget(part);
		topLayout->addWidget(comp);

		// extension widget
		QPushButton* allButton = new QPushButton("All");
		QPushButton* noneButton = new QPushButton("None");

		QVBoxLayout* vl = new QVBoxLayout;
		vl->addWidget(pc[ 0] = new QCheckBox("Module"               ));
		vl->addWidget(pc[ 1] = new QCheckBox("Control"              ));
		vl->addWidget(pc[ 2] = new QCheckBox("Globals"              ));
		vl->addWidget(pc[ 3] = new QCheckBox("Material"             ));
		vl->addWidget(pc[ 4] = new QCheckBox("Geometry"             ));
		vl->addWidget(pc[ 5] = new QCheckBox("MeshData  (FEBio 2.5)"));
		vl->addWidget(pc[ 6] = new QCheckBox("Boundary"             ));
		vl->addWidget(pc[ 7] = new QCheckBox("Loads"                ));
		vl->addWidget(pc[ 8] = new QCheckBox("Initial"              ));
		vl->addWidget(pc[ 9] = new QCheckBox("Constraints"          ));
		vl->addWidget(pc[10] = new QCheckBox("Contact  (FEBio 2.x)" ));
		vl->addWidget(pc[11] = new QCheckBox("Discrete (FEBio 2.x)" ));
		vl->addWidget(pc[12] = new QCheckBox("LoadData"             ));
		vl->addWidget(pc[13] = new QCheckBox("Output"               ));
		vl->addWidget(pc[14] = new QCheckBox("Step"                 ));

		QVBoxLayout* buttonLayout = new QVBoxLayout;
		buttonLayout->addWidget(allButton);
		buttonLayout->addWidget(noneButton);
		buttonLayout->addStretch();

		QHBoxLayout* extLayout = new QHBoxLayout;
		extLayout->addLayout(vl);
		extLayout->addLayout(buttonLayout);

		QGroupBox* pg = new QGroupBox("Include sections");
		pg->setLayout(extLayout);

		// button box
		QDialogButtonBox* bb = new QDialogButtonBox(Qt::Vertical);
		QPushButton* okButton     = bb->addButton("Ok"    , QDialogButtonBox::ActionRole); okButton->setDefault(true);
		QPushButton* cancelButton = bb->addButton("Cancel", QDialogButtonBox::ActionRole);
		QPushButton* moreButton = new QPushButton("More");
		moreButton->setCheckable(true);
		bb->addButton(moreButton, QDialogButtonBox::ActionRole);

		// signals
		QObject::connect(okButton, SIGNAL(clicked()), parent, SLOT(accept()));
		QObject::connect(cancelButton, SIGNAL(clicked()), parent, SLOT(reject()));
		QObject::connect(moreButton, SIGNAL(toggled(bool)), pg, SLOT(setVisible(bool)));
		QObject::connect(allButton, SIGNAL(clicked()), parent, SLOT(OnAllClicked()));
		QObject::connect(noneButton, SIGNAL(clicked()), parent, SLOT(OnNoneClicked()));

		// construct layout
		QGridLayout* mainLayout = new QGridLayout;
		mainLayout->setSizeConstraint(QLayout::SetFixedSize);
		mainLayout->addLayout(topLayout, 0, 0, 1, 1, (Qt::AlignTop | Qt::AlignVCenter));
		mainLayout->addWidget(bb, 0, 1);
		mainLayout->addWidget(pg, 1, 0, 1, 2);

		parent->setLayout(mainLayout);

		pg->hide();
	}
};

CDlgExportFEBio::CDlgExportFEBio(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgExportFEBio)
{
	ui->setupUi(this);
	m_bexportParts = false;
	m_compress = false;
	m_bexportSelections = false;

	m_nversion = 2;
	for (int i=0; i<MAX_SECTIONS; ++i) 
	{
		m_nsection[i] = true;
		ui->pc[i]->setChecked(true);
	}
}

void CDlgExportFEBio::accept()
{
	m_nversion = ui->combo->currentIndex();
	m_bexportParts = ui->part->isChecked();
	m_compress = ui->comp->isChecked();
	m_bexportSelections = ui->sel->isChecked();

	for (int i=0; i<MAX_SECTIONS; ++i) m_nsection[i] = ui->pc[i]->isChecked();

	QDialog::accept();
}

void CDlgExportFEBio::OnAllClicked()
{
	for (int i=0; i<MAX_SECTIONS; ++i) ui->pc[i]->setChecked(true);
}

void CDlgExportFEBio::OnNoneClicked()
{
	for (int i = 0; i<MAX_SECTIONS; ++i) ui->pc[i]->setChecked(false);
}
