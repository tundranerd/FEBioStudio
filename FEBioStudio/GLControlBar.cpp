#include "stdafx.h"
#include "GLControlBar.h"
#include "MainWindow.h"
#include "Document.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QValidator>
#include <QToolButton>
#include <QButtonGroup>
#include <QValidator>
#include <QDoubleSpinBox>
#include <GeomLib/GCurveMeshObject.h>

class CGLControlBar_UI
{
public:
	QLineEdit*	x;
	QLineEdit*	y;
	QLineEdit*	z;

	CMainWindow*	m_wnd;

	QWidget* edit;

	QToolButton*	but[4];
	QToolButton*	selConnect;
	QDoubleSpinBox*	maxAngle;
	QToolButton*	selPath;
	QToolButton*	cull;

public:
	void setup(CGLControlBar* bar)
	{
		QHBoxLayout* h = new QHBoxLayout;
		h->setMargin(0);

		QToolButton* b  = addButton(QIcon(":/icons/pivot.png"), "Lock pivot");
		QToolButton* b1 = addButton(QIcon(":/icons/snaptogrid.png"), "Snap to grid"); b1->setChecked(true);
		QToolButton* b2 = addButton(QIcon(":/icons/snaptonode.png"), "Snap to node");
		QToolButton* b3 = addButton(QIcon(":/icons/hide.png"), "Hide selection", false);
		QToolButton* b4 = addButton(QIcon(":/icons/show.png"), "Show all", false);
		QToolButton* b5 = addButton(QIcon(":/icons/toggle_visible.png"), "Toggle visibility", false);
		QToolButton* b6 = addButton(QIcon(":/icons/zoom_select.png"), "Zoom to selection", false); 
		QToolButton* b7 = addButton(QIcon(":/icons/zoom_all.png"), "Zoom to extents", false);

		// mesh editing tool buttons
		edit = new QWidget;

		but[0] = addButton(QIcon(":/icons/selElem.png"), "Select elements"); 
		but[1] = addButton(QIcon(":/icons/selFace.png"), "Select faces");
		but[2] = addButton(QIcon(":/icons/selEdge.png"), "Select edges");
		but[3] = addButton(QIcon(":/icons/selNode.png"), "Select nodes");

		selConnect = addButton(QIcon(":/icons/select_connected.png"), "Select connected");

		maxAngle = new QDoubleSpinBox; maxAngle->setRange(0.0, 180); maxAngle->setSingleStep(0.5);
		maxAngle->setMaximumWidth(60);

		selPath = addButton(QIcon(":/icons/select_path.png"), "Select by closest path");
		cull    = addButton(QIcon(":/icons/backface.png"), "Select backfacing");

		QHBoxLayout* hl = new QHBoxLayout;
		hl->setMargin(0);

		QFrame* sep = new QFrame;
		sep->setFrameShape(QFrame::VLine);
//		sep->setFrameShadow(QFrame::Sunken);

		hl->addWidget(sep);
		hl->addWidget(but[0]);
		hl->addWidget(but[1]);
		hl->addWidget(but[2]);
		hl->addWidget(but[3]);
		hl->addWidget(selConnect);
		hl->addWidget(maxAngle);
		hl->addWidget(selPath);
		hl->addWidget(cull);
		hl->addStretch();

		QButtonGroup* bg = new QButtonGroup(bar);
		bg->setExclusive(false);
		bg->addButton(but[0], 0);
		bg->addButton(but[1], 1);
		bg->addButton(but[2], 2);
		bg->addButton(but[3], 3);

		edit->setLayout(hl);

		// assemble tools
		h->addWidget(b);

		h->addWidget(x = new QLineEdit); x->setMaximumWidth(100); x->setValidator(new QDoubleValidator); x->setReadOnly(true);
		h->addWidget(y = new QLineEdit); y->setMaximumWidth(100); y->setValidator(new QDoubleValidator); y->setReadOnly(true);
		h->addWidget(z = new QLineEdit); z->setMaximumWidth(100); z->setValidator(new QDoubleValidator); z->setReadOnly(true);

		h->addWidget(b1);
		h->addWidget(b2);
		h->addWidget(b3);
		h->addWidget(b4);
		h->addWidget(b5);
		h->addWidget(b6);
		h->addWidget(b7);
		h->addWidget(edit);
		h->addStretch();

		bar->setLayout(h);

		QObject::connect(x, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(y, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(z, SIGNAL(textEdited(const QString&)), bar, SLOT(onPivotChanged()));
		QObject::connect(b, SIGNAL(clicked(bool)), bar, SLOT(onPivotClicked(bool)));
		QObject::connect(b1, SIGNAL(clicked(bool)), bar, SLOT(onSnapToGridClicked(bool)));
		QObject::connect(b2, SIGNAL(clicked(bool)), bar, SLOT(onSnapToNodeClicked(bool)));
		QObject::connect(b3, SIGNAL(clicked(bool)), bar, SLOT(onHideSelection(bool)));
		QObject::connect(b4, SIGNAL(clicked(bool)), bar, SLOT(onShowAll(bool)));
		QObject::connect(b5, SIGNAL(clicked(bool)), bar, SLOT(onToggleVisibleClicked(bool)));
		QObject::connect(b6, SIGNAL(clicked(bool)), bar, SLOT(onZoomSelectClicked(bool)));
		QObject::connect(b7, SIGNAL(clicked(bool)), bar, SLOT(onZoomAllClicked(bool)));
		QObject::connect(bg, SIGNAL(buttonClicked(int)), bar, SLOT(onMeshButtonClicked(int)));
		QObject::connect(selConnect, SIGNAL(clicked(bool)), bar, SLOT(onSelectConnected(bool)));
		QObject::connect(selPath, SIGNAL(clicked(bool)), bar, SLOT(onSelectClosestPath(bool)));
		QObject::connect(maxAngle, SIGNAL(valueChanged(double)), bar, SLOT(onMaxAngleChanged(double)));
		QObject::connect(cull, SIGNAL(clicked(bool)), bar, SLOT(onSelectBackfacing(bool)));
	}

	QToolButton* addButton(const QIcon& icon, const QString& toolTip, bool isCheckable = true)
	{
		QToolButton* b = new QToolButton;
		b->setIcon(icon);
		b->setCheckable(isCheckable);
		b->setAutoRaise(true);
		b->setToolTip(QString("<font color=\"black\">") + toolTip);
		return b;
	}

	vec3d getPivot()
	{
		vec3d r;
		r.x = x->text().toDouble();
		r.y = y->text().toDouble();
		r.z = z->text().toDouble();
		return r;
	}

	void showEditButtons(bool b, bool showElem = true, bool showFace = true)
	{
		edit->setVisible(b);
		if (b)
		{
			but[0]->setEnabled(showElem);
			but[1]->setEnabled(showFace);
		}
	}

	void checkButton(int id)
	{
		for (int i = 0; i<4; ++i)
			if (id != i) but[i]->setChecked(false);

		if (id == -1) return;
		if (but[id]->isChecked() == false) 
		{
			but[id]->blockSignals(true);
			but[id]->setChecked(true);
			but[id]->blockSignals(false);
		}
	}
};

CGLControlBar::CGLControlBar(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new CGLControlBar_UI)
{
	ui->m_wnd = wnd;
	ui->setup(this);
	ui->showEditButtons(false);
}

void CGLControlBar::Update()
{
	CGLView* view = ui->m_wnd->GetGLView();
	
	vec3d pivot = view->GetPivotPosition();
	ui->x->setText(QString::number(pivot.x));	
	ui->y->setText(QString::number(pivot.y));
	ui->z->setText(QString::number(pivot.z));

	bool pivotMode = view->GetPivotMode();

	ui->x->setReadOnly(!pivotMode);
	ui->y->setReadOnly(!pivotMode);
	ui->z->setReadOnly(!pivotMode);

	CDocument* pdoc = ui->m_wnd->GetDocument();

	GSurfaceMeshObject* ps = dynamic_cast<GSurfaceMeshObject*>(pdoc->GetActiveObject());
	if (ps)
	{
		ui->showEditButtons(true, false);
		int item = pdoc->GetItemMode();
		if (item == ITEM_ELEM) { pdoc->SetItemMode(ITEM_MESH); item = ITEM_MESH; }
		switch (item)
		{
		case ITEM_MESH: ui->checkButton(-1); break;
//		case ITEM_ELEM: ui->checkButton(0); break;
		case ITEM_FACE: ui->checkButton(1); break;
		case ITEM_EDGE: ui->checkButton(2); break;
		case ITEM_NODE: ui->checkButton(3); break;
		}

		VIEW_SETTINGS& view = pdoc->GetViewSettings();
		ui->selConnect->setChecked(view.m_bconn);
		ui->selPath->setChecked(view.m_bselpath);
		ui->maxAngle->setValue(view.m_fconn);
		ui->cull->setChecked(!view.m_bcullSel);

		return;
	}

	GCurveMeshObject* pc = dynamic_cast<GCurveMeshObject*>(pdoc->GetActiveObject());
	if (pc)
	{
		ui->showEditButtons(true, false, false);
		int item = pdoc->GetItemMode();
		if (item == ITEM_ELEM) { pdoc->SetItemMode(ITEM_MESH); item = ITEM_MESH; }
		switch (item)
		{
		case ITEM_MESH: ui->checkButton(-1); break;
//		case ITEM_ELEM: ui->checkButton(0); break;
//		case ITEM_FACE: ui->checkButton(1); break;
		case ITEM_EDGE: ui->checkButton(2); break;
		case ITEM_NODE: ui->checkButton(3); break;
		}

		VIEW_SETTINGS& view = pdoc->GetViewSettings();
		ui->selConnect->setChecked(view.m_bconn);
		ui->selPath->setChecked(view.m_bselpath);
		ui->maxAngle->setValue(view.m_fconn);
		ui->cull->setChecked(!view.m_bcullSel);

		return;
	}

	GObject* po = pdoc->GetActiveObject();
	if (po && po->GetFEMesh())
	{
		ui->showEditButtons(true);

		int item = pdoc->GetItemMode();
		switch (item)
		{
		case ITEM_MESH: ui->checkButton(-1); break;
		case ITEM_ELEM: ui->checkButton(0); break;
		case ITEM_FACE: ui->checkButton(1); break;
		case ITEM_EDGE: ui->checkButton(2); break;
		case ITEM_NODE: ui->checkButton(3); break;
		}

		VIEW_SETTINGS& view = pdoc->GetViewSettings();
		ui->selConnect->setChecked(view.m_bconn);
		ui->selPath->setChecked(view.m_bselpath);
		ui->maxAngle->setValue(view.m_fconn);
		ui->cull->setChecked(!view.m_bcullSel);

		return;
	}

	ui->showEditButtons(false);
}

void CGLControlBar::onPivotChanged()
{
	vec3d p = ui->getPivot();
	CGLView* view = ui->m_wnd->GetGLView();
	view->SetPivot(p);
}

void CGLControlBar::onPivotClicked(bool b)
{
	CGLView* view = ui->m_wnd->GetGLView();
	view->SetPivotMode(b);
	Update();
}

void CGLControlBar::onSnapToGridClicked(bool b)
{
	CDocument* doc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = doc->GetViewSettings();
	view.m_snapToGrid = b;
}

void CGLControlBar::onSnapToNodeClicked(bool b)
{
	CDocument* doc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = doc->GetViewSettings();
	view.m_snapToNode = b;
}

void CGLControlBar::onToggleVisibleClicked(bool b)
{
	ui->m_wnd->on_actionToggleVisible_triggered();
}

void CGLControlBar::onHideSelection(bool b)
{
	ui->m_wnd->on_actionHideSelection_triggered();
}

void CGLControlBar::onShowAll(bool b)
{
	ui->m_wnd->on_actionUnhideAll_triggered();
}

void CGLControlBar::onZoomSelectClicked(bool b)
{
	ui->m_wnd->on_actionZoomSelect_triggered();
}

void CGLControlBar::onZoomAllClicked(bool b)
{
	ui->m_wnd->on_actionZoomExtents_triggered();
}

void CGLControlBar::onMeshButtonClicked(int id)
{
	for (int i = 0; i<4; ++i)
		if (id != i) ui->but[i]->setChecked(false);

	if (ui->but[id]->isChecked() == false) id = -1;

	int newMode = 0;
	switch (id)
	{
	case -1: newMode = ITEM_MESH; break;
	case 0: newMode = ITEM_ELEM; break;
	case 1: newMode = ITEM_FACE; break;
	case 2: newMode = ITEM_EDGE; break;
	case 3: newMode = ITEM_NODE; break;
	}

	CDocument* pdoc = ui->m_wnd->GetDocument();
	pdoc->SetItemMode(newMode);

	ui->m_wnd->RedrawGL();
}

void CGLControlBar::onSelectConnected(bool b)
{
	CDocument* pdoc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bconn = b;
}

void CGLControlBar::onSelectClosestPath(bool b)
{
	CDocument* pdoc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bselpath = b;
}

void CGLControlBar::onMaxAngleChanged(double v)
{
	CDocument* pdoc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_fconn = v;
}

void CGLControlBar::onSelectBackfacing(bool b)
{
	CDocument* pdoc = ui->m_wnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	view.m_bcullSel = !b;
}
