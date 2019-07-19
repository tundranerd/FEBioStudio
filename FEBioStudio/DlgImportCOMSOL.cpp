#include "stdafx.h"
#include "DlgImportCOMSOL.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class Ui::CDlgImportCOMSOL
{
public:
	QCheckBox* pc[9];

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;

		pc[0] = new QCheckBox("Convert domains to named selected");
		pc[1] = new QCheckBox("Auto-partition domain into parts");
		pc[2] = new QCheckBox("Segregate element types");
		pc[3] = new QCheckBox("Import TRI elements");
		pc[4] = new QCheckBox("Import QUAD elements");
		pc[5] = new QCheckBox("Import TET elements");
		pc[6] = new QCheckBox("Import PRISM elements");
		pc[7] = new QCheckBox("Import HEX elements");
		pc[8] = new QCheckBox("Import PYR elements (as 2 TETS each)");

		for (int i=0; i<9; ++i) lo->addWidget(pc[i]);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};


CDlgImportCOMSOL::CDlgImportCOMSOL(COMSOLimport* fileReader, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgImportCOMSOL)
{
	m_fileReader = fileReader;
	ui->setupUi(this);

	ui->pc[0]->setChecked(m_fileReader->m_domainstosets);
	ui->pc[1]->setChecked(m_fileReader->m_bautopart    );
	ui->pc[2]->setChecked(m_fileReader->m_eltypeseg    );
	ui->pc[3]->setChecked(m_fileReader->m_addtris      );
	ui->pc[4]->setChecked(m_fileReader->m_addquads     );
	ui->pc[5]->setChecked(m_fileReader->m_addtets      );
	ui->pc[6]->setChecked(m_fileReader->m_addprisms    );
	ui->pc[7]->setChecked(m_fileReader->m_addhexes     );
	ui->pc[8]->setChecked(m_fileReader->m_pyrstotets   );
}

void CDlgImportCOMSOL::accept()
{
	m_fileReader->m_domainstosets = (ui->pc[0]->isChecked());
	m_fileReader->m_bautopart     = (ui->pc[1]->isChecked());
	m_fileReader->m_eltypeseg     = (ui->pc[2]->isChecked());
	m_fileReader->m_addtris       = (ui->pc[3]->isChecked());
	m_fileReader->m_addquads      = (ui->pc[4]->isChecked());
	m_fileReader->m_addtets       = (ui->pc[5]->isChecked());
	m_fileReader->m_addprisms     = (ui->pc[6]->isChecked());
	m_fileReader->m_addhexes      = (ui->pc[7]->isChecked());
	m_fileReader->m_pyrstotets    = (ui->pc[8]->isChecked());

	QDialog::accept();
}
