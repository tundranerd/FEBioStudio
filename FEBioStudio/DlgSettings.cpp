#include "stdafx.h"
#include "DlgSettings.h"
#include "MainWindow.h"
#include "Document.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include "PropertyList.h"
#include "PropertyListForm.h"
#include "PropertyListView.h"
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include <PostLib/Palette.h>

//-----------------------------------------------------------------------------
class CBackgroundProps : public CDataPropertyList
{
public:
	CBackgroundProps()
	{
		addColorProperty(&m_bg1, "Background color 1");
		addColorProperty(&m_bg2, "Background color 2");
		addColorProperty(&m_fg, "Foreground color");

		QStringList styles;
		styles << "Color 1" << "Color 2" << "Horizontal" << "Vertical";
		addEnumProperty(&m_nstyle, "Background style")->setEnumValues(styles);

		m_nstyle = 0;
	}

public:
	QColor m_bg1, m_bg2;
	QColor	m_fg;
	int		m_nstyle;
};

//-----------------------------------------------------------------------------
class CDisplayProps : public CDataPropertyList
{
public:
	CDisplayProps()
	{
		addDoubleProperty(&m_nodeSize, "Size of nodes");
		addDoubleProperty(&m_lineSize, "Line width");
		addColorProperty (&m_meshColor, "Mesh color"  );
		addBoolProperty  (&m_bnormal, "Show normals"  );
		addDoubleProperty(&m_scaleNormal, "Normals scale factor");
        QStringList vconv;
        vconv <<"First-angle projection (XZ)"<<"First-angle projection (XY)"<<"Third-angle projection (XY)";
        addEnumProperty(&m_nconv, "Multiview projection")->setEnumValues(vconv);
		addEnumProperty(&m_ntrans, "Object transparency mode")->setEnumValues(QStringList() << "None" << "Selected only" << "Unselected only");
		addEnumProperty(&m_nobjcol, "Object color")->setEnumValues(QStringList() << "Default" << "Object");
	}

public:
	double	m_nodeSize;
	double	m_lineSize;
	QColor  m_meshColor;
	bool	m_bnormal;
	double	m_scaleNormal;
    int     m_nconv;
	int		m_ntrans;
	int		m_nobjcol;
};

//-----------------------------------------------------------------------------
class CPhysicsProps : public CDataPropertyList
{
public:
	CPhysicsProps()
	{
		addBoolProperty  (&m_showRigidJoints, "Show rigid joints");
		addBoolProperty  (&m_showRigidWalls , "Show rigid walls" );
		addBoolProperty  (&m_showFibers     , "Show material fibers");
		addDoubleProperty(&m_fiberScale   , "Fiber scale factor"  );
		addBoolProperty  (&m_showMatAxes  ,"Show material axes"   );
		addBoolProperty  (&m_showHiddenFibers, "Show fibers/axes on hidden parts");
	}

public:
	bool	m_showRigidJoints;
	bool	m_showRigidWalls;
	bool	m_showFibers;
	bool	m_showMatAxes;
	double	m_fiberScale;
	bool	m_showHiddenFibers;
};

//-----------------------------------------------------------------------------
class CUIProps : public CDataPropertyList
{
public:
	CUIProps(QDialog* parent, CMainWindow* wnd) : m_dlg(parent), m_wnd(wnd)
	{
		addBoolProperty(&m_apply, "Emulate apply action");
		addBoolProperty(&m_bcmd , "Clear undo stack on save");
		QStringList themes  = QStringList() << "Default" << "Dark";
#ifdef LINUX
		themes << "Adwaita" << "Adwaita Dark";
#endif
		addEnumProperty(&m_theme, "Theme")->setEnumValues(themes);
		addProperty("Recent projects list", CProperty::Action)->info = QString("Clear");
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		if (i == 3)
		{
			if (QMessageBox::question(m_dlg, "FEBio Studio", "Are you sure you want to clear the recent project list.\nThis can not be undone!"))
			{
				m_wnd->ClearRecentProjectsList();
			}
		}
		else CDataPropertyList::SetPropertyValue(i, v);
	}

public:
	QDialog*	m_dlg;
	CMainWindow* m_wnd;
	bool	m_apply;
	bool	m_bcmd;
	int		m_theme;
};

//-----------------------------------------------------------------------------
class CSelectionProps : public CDataPropertyList
{
public:
	CSelectionProps()
	{
		addBoolProperty(&m_bconnect, "Select connected");
		addEnumProperty(&m_ntagInfo, "Tag info")->setEnumValues(QStringList() << "Item numbers" << "Item numbers and connecting nodes");
		addBoolProperty(&m_backface, "Ignore backfacing items");
		addBoolProperty(&m_binterior, "Ignore interior items");
		addBoolProperty(&m_bpart    , "Respect partitions");
		m_bconnect = false;
		m_ntagInfo = 0;
	}

public:
	bool	m_bconnect;
	int		m_ntagInfo;
	bool	m_backface;
	bool	m_binterior;
	bool	m_bpart;
};

//-----------------------------------------------------------------------------
class CLightingProps : public CPropertyList
{
public:
	CLightingProps()
	{
		addProperty("Enable lighting", CProperty::Bool);
		addProperty("Diffuse intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Ambient intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Render shadows", CProperty::Bool);
		addProperty("Shadow intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Light direction"  , CProperty::Vec3);

		m_blight = true;
		m_diffuse = 0.7f;
		m_ambient = 0.3f;
		m_bshadow = false;
		m_shadow = 0.1f;
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: return m_blight; break;
		case 1: return m_diffuse; break;
		case 2: return m_ambient; break;
		case 3: return m_bshadow; break;
		case 4: return m_shadow; break;
		case 5: return vecToString(m_pos); break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_blight = v.toBool(); break;
		case 1: m_diffuse = v.toFloat(); break;
		case 2: m_ambient = v.toFloat(); break;
		case 3: m_bshadow = v.toBool(); break;
		case 4: m_shadow = v.toFloat(); break;
		case 5: m_pos = stringToVec(v.toString()); break;
		}
	}

public:
	bool	m_blight;
	float	m_diffuse;
	float	m_ambient;
	bool	m_bshadow;
	float	m_shadow;
	vec3f	m_pos;
};

//-----------------------------------------------------------------------------
class CCameraProps : public CPropertyList
{
public:
	CCameraProps()
	{
		addProperty("Animate camera", CProperty::Bool);
		addProperty("Animation speed", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Animation bias", CProperty::Float)->setFloatRange(0.0, 1.0);

		m_banim = true;
		m_speed = 0.8f;
		m_bias = 0.8f;
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: return m_banim; break;
		case 1: return m_speed; break;
		case 2: return m_bias; break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_banim = v.toBool(); break;
		case 1: m_speed = v.toFloat(); break;
		case 2: m_bias = v.toFloat(); break;
		}
	}

public:
	bool	m_banim;
	float	m_speed;
	float	m_bias;
};

//=================================================================================================
ColorGradient::ColorGradient(QWidget* parent) : QWidget(parent)
{
	m_map.jet();
}

QSize ColorGradient::sizeHint() const
{
	return QSize(20, 20);
}

void ColorGradient::setColorMap(const Post::CColorMap& m)
{
	m_map = m;
	repaint();
}

void ColorGradient::paintEvent(QPaintEvent* ev)
{
	QRect rt = rect();
	QPainter p(this);

	int r = 3;
	int x0 = rt.left();
	int x1 = rt.right();
	int y0 = rt.top();
	int y1 = y0 + 2 * r;
	int y2 = rt.bottom();
	int w = rt.width();

	p.fillRect(x0, y0, w, y1 - y0, Qt::white);

	int ncol = m_map.Colors();
	for (int i = 0; i<ncol; ++i)
	{
		float x = m_map.GetColorPos(i);
		int xi = x0 + (int)(w*x) - r;
		p.setPen(Qt::gray);
		p.setBrush(Qt::gray);
		p.drawEllipse(xi, y0, 2 * r, 2 * r);
	}

	for (int i = 0; i<rt.width(); ++i)
	{
		float w = (float)i / rt.width();
		GLColor c = m_map.map(w);
		p.setPen(QColor(c.r, c.g, c.b));
		p.drawLine(x0 + i, y1, x0 + i, y2);
	}
}

//=================================================================================================
//-----------------------------------------------------------------------------
class CPaletteWidget : public QWidget
{
public:
	QComboBox*	pal;

public:
	CPaletteWidget(QWidget* parent = 0) : QWidget(parent)
	{
		QLabel* label = new QLabel("Current palette:");
		label->setFixedWidth(100);
		label->setAlignment(Qt::AlignRight | Qt::AlignCenter);
		pal = new QComboBox; label->setBuddy(label);

		QHBoxLayout* h0 = new QHBoxLayout;
		h0->addWidget(label);
		h0->addWidget(pal);

		QPushButton* load = new QPushButton("Load Palette ..."); load->setObjectName("load");
		QPushButton* save = new QPushButton("Save Palette ..."); save->setObjectName("save");
		QPushButton* create = new QPushButton("Create palette from materials ..."); create->setObjectName("create");
		QPushButton* apply = new QPushButton("Apply palette to materials ..."); apply->setObjectName("apply");

		QVBoxLayout* buttons = new QVBoxLayout;
		buttons->addWidget(load);
		buttons->addWidget(save);
		buttons->addWidget(create);
		buttons->addWidget(apply);

		QHBoxLayout* h1 = new QHBoxLayout;
		h1->addStretch();
		h1->addLayout(buttons);

		QVBoxLayout* pl = new QVBoxLayout;
		pl->addLayout(h0);
		pl->addLayout(h1);
		pl->addStretch();

		setLayout(pl);
	}
};

//-----------------------------------------------------------------------------
CColormapWidget::CColormapWidget(QWidget* parent) : QWidget(parent)
{
	QHBoxLayout* h = new QHBoxLayout;
	QComboBox* l = m_maps = new QComboBox;
	l->setMinimumWidth(120);
	l->setCurrentIndex(0);

	QPushButton* newButton = new QPushButton("New ...");
	QPushButton* delButton = new QPushButton("Delete");
	QPushButton* editButton = new QPushButton("Edit ...");
	h->addWidget(new QLabel("Select color map:"));
	h->addWidget(l);
	h->addWidget(newButton);
	h->addWidget(delButton);
	h->addWidget(editButton);
	h->addStretch();

	QVBoxLayout* v = new QVBoxLayout;
	v->addLayout(h);

	h = new QHBoxLayout;

	m_spin = new QSpinBox;
	m_spin->setRange(2, 9);
	m_spin->setMaximumWidth(75);
	m_spin->setMinimumWidth(50);

	m_default = new QCheckBox("Set as default");

	h->addWidget(new QLabel("Number of colors:"));
	h->addWidget(m_spin);
	h->addStretch();
	v->addWidget(m_default);
	v->addLayout(h);

	m_grid = new QGridLayout;

	v->addLayout(m_grid);

	QPushButton* invertButton = new QPushButton("Invert");
	h = new QHBoxLayout();
	h->addStretch();
	h->addWidget(invertButton);
	v->addLayout(h);
	v->addStretch();

	h = new QHBoxLayout();
	h->addWidget(m_grad = new ColorGradient);
	v->addLayout(h);

	setLayout(v);

	QObject::connect(l, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMapChanged(int)));
	QObject::connect(m_spin, SIGNAL(valueChanged(int)), this, SLOT(onSpinValueChanged(int)));
	QObject::connect(newButton, SIGNAL(clicked()), this, SLOT(onNew()));
	QObject::connect(delButton, SIGNAL(clicked()), this, SLOT(onDelete()));
	QObject::connect(editButton, SIGNAL(clicked()), this, SLOT(onEdit()));
	QObject::connect(invertButton, SIGNAL(clicked()), this, SLOT(onInvert()));
	QObject::connect(m_default, SIGNAL(stateChanged(int)), this, SLOT(onSetDefault(int)));

	updateMaps();
	m_currentMap = 0;
	currentMapChanged(0);
}

void CColormapWidget::updateMaps()
{
	m_maps->clear();
	for (int i = 0; i < Post::ColorMapManager::ColorMaps(); ++i)
	{
		string name = Post::ColorMapManager::GetColorMapName(i);
		m_maps->addItem(QString(name.c_str()));
	}
}

void CColormapWidget::onSetDefault(int nstate)
{
	int n = m_maps->currentIndex();
	Post::ColorMapManager::SetDefaultMap(n);
	m_default->setDisabled(true);
}

void CColormapWidget::onNew()
{
	int n = Post::ColorMapManager::UserColorMaps() + 1;
	QString name = QString("user%1").arg(n);
	bool bok = true;
	QString newName = QInputDialog::getText(this, "New color map", "name:", QLineEdit::Normal, name, &bok);
	if (bok && (newName.isEmpty() == false))
	{
		Post::CColorMap& map = Post::ColorMapManager::GetColorMap(m_currentMap);
		string sname = newName.toStdString();
		Post::ColorMapManager::AddColormap(sname, map);

		updateMaps();
		m_maps->setCurrentIndex(Post::ColorMapManager::ColorMaps() - 1);
	}
}

void CColormapWidget::onDelete()
{
	if (Post::ColorMapManager::RemoveColormap(m_currentMap) == false)
	{
		QMessageBox::critical(this, "Delete Colormap", "Cannot delete default color maps.");
	}
	else
	{
		m_maps->removeItem(m_currentMap);
	}
}

void CColormapWidget::onEdit()
{
	string sname = Post::ColorMapManager::GetColorMapName(m_currentMap);
	QString name = QString::fromStdString(sname);
	bool bok = true;
	QString newName = QInputDialog::getText(this, "Edit color map", "name:", QLineEdit::Normal, name, &bok);
	if (bok && (newName.isEmpty() == false))
	{
		Post::ColorMapManager::SetColorMapName(m_currentMap, newName.toStdString());
		m_maps->setItemText(m_currentMap, newName);
	}
}

void CColormapWidget::onInvert()
{
	Post::CColorMap& map = Post::ColorMapManager::GetColorMap(m_currentMap);
	map.Invert();
	updateColorMap(map);
}

void CColormapWidget::updateColorMap(const Post::CColorMap& map)
{
	clearGrid();

	m_spin->setValue(map.Colors());

	QLineEdit* l;
	CColorButton* b;
	for (int i = 0; i<map.Colors(); ++i)
	{
		QColor c = toQColor(map.GetColor(i));
		float f = map.GetColorPos(i);

		m_grid->addWidget(new QLabel(QString("Color %1").arg(i + 1)), i, 0, Qt::AlignRight);
		m_grid->addWidget(l = new QLineEdit, i, 1); l->setValidator(new QDoubleValidator); l->setText(QString::number(f)); l->setMaximumWidth(100);
		m_grid->addWidget(b = new CColorButton, i, 2); b->setColor(c);

		QObject::connect(l, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
		QObject::connect(b, SIGNAL(colorChanged(QColor)), this, SLOT(onDataChanged()));
	}

	m_grad->setColorMap(map);
}

void CColormapWidget::clearGrid()
{
	while (m_grid->count())
	{
		QLayoutItem* item = m_grid->takeAt(0);
		delete item->widget();
		delete item;
	}
}

void CColormapWidget::currentMapChanged(int n)
{
	if (n != -1)
	{
		m_currentMap = n;
		updateColorMap(Post::ColorMapManager::GetColorMap(n));

		int defaultMap = Post::ColorMapManager::GetDefaultMap();
		if (n == defaultMap)
		{
			m_default->setChecked(true);
			m_default->setDisabled(true);
		}
		else
		{
			m_default->setChecked(false);
			m_default->setEnabled(true);
		}
	}
	else m_default->setDisabled(true);
}

void CColormapWidget::onDataChanged()
{
	Post::CColorMap& map = Post::ColorMapManager::GetColorMap(m_currentMap);
	for (int i = 0; i<map.Colors(); ++i)
	{
		QLineEdit* pos = dynamic_cast<QLineEdit*>(m_grid->itemAtPosition(i, 1)->widget()); assert(pos);
		if (pos)
		{
			float f = pos->text().toFloat();
			map.SetColorPos(i, f);
		}

		CColorButton* col = dynamic_cast<CColorButton*>(m_grid->itemAtPosition(i, 2)->widget()); assert(col);
		if (col)
		{
			QColor c = col->color();
			map.SetColor(i, toGLColor(c));
		}
	}
	m_grad->setColorMap(map);
}

void CColormapWidget::onSpinValueChanged(int n)
{
	Post::CColorMap& map = Post::ColorMapManager::GetColorMap(m_currentMap);
	if (map.Colors() != n)
	{
		map.SetColors(n);
		updateColorMap(map);
	}
}

//-----------------------------------------------------------------------------
class Ui::CDlgSettings
{
public:
	CBackgroundProps*	m_bg;
	CDisplayProps*		m_display;
	CPhysicsProps*		m_physics;
	CUIProps*			m_ui;
	QDialogButtonBox*	buttonBox;
	CPaletteWidget*		m_pal;
	CColormapWidget*	m_map;
	CSelectionProps*	m_select;
	CLightingProps*		m_light;
	CCameraProps*		m_cam;

	::CPropertyListView*	bg_panel;
	::CPropertyListView*	di_panel;
	::CPropertyListView*	ph_panel;
	::CPropertyListForm*	ui_panel;
	::CPropertyListView*	se_panel;
	::CPropertyListView*	li_panel;
	::CPropertyListView*	ca_panel;

public:
	CDlgSettings(QDialog* parent, ::CMainWindow* wnd)
	{
		m_bg = new CBackgroundProps;
		m_display = new CDisplayProps;
		m_physics = new CPhysicsProps;
		m_ui = new CUIProps(parent, wnd);
		m_pal = new CPaletteWidget;
		m_map = new CColormapWidget;
		m_select = new CSelectionProps;
		m_light = new CLightingProps;
		m_cam = new CCameraProps;
	}

	void setupUi(::CDlgSettings* pwnd)
	{
		bg_panel = new ::CPropertyListView;
		di_panel = new ::CPropertyListView;
		ph_panel = new ::CPropertyListView;
		ui_panel = new ::CPropertyListForm;
		se_panel = new ::CPropertyListView;
		li_panel = new ::CPropertyListView;
		ca_panel = new ::CPropertyListView;

		QStackedWidget* stack = new QStackedWidget;
		QListWidget* list = new QListWidget;

		stack->addWidget(bg_panel); list->addItem("Background");
		stack->addWidget(ca_panel); list->addItem("Camera");
		stack->addWidget(m_map   ); list->addItem("Colormap");
		stack->addWidget(di_panel); list->addItem("Display");
		stack->addWidget(li_panel); list->addItem("Lighting");
		stack->addWidget(m_pal   ); list->addItem("Palette");
		stack->addWidget(ph_panel); list->addItem("Physics");
		stack->addWidget(se_panel); list->addItem("Selection");
		stack->addWidget(ui_panel); list->addItem("UI");
		list->setResizeMode(QListView::ResizeMode::Adjust);

		QHBoxLayout* hl = new QHBoxLayout;
		hl->setMargin(0);
		hl->addWidget(list);
		hl->addWidget(stack);
		
		QVBoxLayout* pg = new QVBoxLayout(pwnd);
		pg->addLayout(hl);

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
		pg->addWidget(buttonBox);

		QObject::connect(buttonBox, SIGNAL(accepted()), pwnd, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), pwnd, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), pwnd, SLOT(onClicked(QAbstractButton*)));
		QObject::connect(list, SIGNAL(currentRowChanged(int)), stack, SLOT(setCurrentIndex(int)));
	}
};


CDlgSettings::CDlgSettings(CMainWindow* pwnd) : ui(new Ui::CDlgSettings(this, pwnd))
{
	m_pwnd = pwnd;
	setWindowTitle("Options");

	CDocument* pdoc = m_pwnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	CGLView* glview = pwnd->GetGLView();

	CGLCamera& cam = glview->GetCamera();

	ui->m_bg->m_bg1 = toQColor(view.m_col1);
	ui->m_bg->m_bg2 = toQColor(view.m_col2);
	ui->m_bg->m_fg = toQColor(view.m_fgcol);
	ui->m_bg->m_nstyle = view.m_nbgstyle;

	ui->m_display->m_meshColor = toQColor(view.m_mcol);
	ui->m_display->m_nodeSize = (double) view.m_node_size;
	ui->m_display->m_lineSize = (double) view.m_line_size;
	ui->m_display->m_bnormal = view.m_bnorm;
	ui->m_display->m_scaleNormal = view.m_scaleNormals;
    ui->m_display->m_nconv = view.m_nconv;
	ui->m_display->m_ntrans = view.m_transparencyMode;
	ui->m_display->m_nobjcol = view.m_objectColor;

	ui->m_physics->m_showRigidJoints = view.m_brigid;
	ui->m_physics->m_showRigidWalls = view.m_bwall;
	ui->m_physics->m_showFibers = view.m_bfiber;
	ui->m_physics->m_fiberScale = view.m_fiber_scale;
	ui->m_physics->m_showMatAxes = view.m_blma;
	ui->m_physics->m_showHiddenFibers = view.m_showHiddenFibers;

	ui->m_ui->m_apply = (view.m_apply == 1);
	ui->m_ui->m_bcmd = view.m_clearUndoOnSave;
	ui->m_ui->m_theme = pwnd->currentTheme();

	ui->m_select->m_bconnect = view.m_bconn;
	ui->m_select->m_ntagInfo = view.m_ntagInfo;
	ui->m_select->m_backface = view.m_bcullSel;
	ui->m_select->m_binterior = view.m_bext;
	ui->m_select->m_bpart = view.m_bpart;

	ui->m_light->m_blight = view.m_bLighting;
	ui->m_light->m_diffuse = view.m_diffuse;
	ui->m_light->m_ambient = view.m_ambient;
	ui->m_light->m_bshadow = view.m_bShadows;
	ui->m_light->m_shadow = view.m_shadow_intensity;
	if (glview) ui->m_light->m_pos = glview->GetLightPosition();

	ui->m_cam->m_banim = true;
	ui->m_cam->m_bias = cam.GetCameraBias();
	ui->m_cam->m_speed = cam.GetCameraSpeed();

	ui->setupUi(this);

	// fill the palette list
	UpdatePalettes();
}

void CDlgSettings::UpdatePalettes()
{
	ui->m_pal->pal->clear();

	Post::CPaletteManager& PM = Post::CPaletteManager::GetInstance();
	int pals = PM.Palettes();
	for (int i = 0; i<pals; ++i)
	{
		ui->m_pal->pal->addItem(QString::fromStdString(PM.Palette(i).Name()));
	}

	ui->m_pal->pal->setCurrentIndex(PM.CurrentIndex());
}

void CDlgSettings::showEvent(QShowEvent* ev)
{
	ui->bg_panel->Update(ui->m_bg);
	ui->di_panel->Update(ui->m_display);
	ui->ph_panel->Update(ui->m_physics);
	ui->ui_panel->setPropertyList(ui->m_ui);
	ui->se_panel->Update(ui->m_select);
	ui->li_panel->Update(ui->m_light);
	ui->ca_panel->Update(ui->m_cam);
}

void CDlgSettings::apply()
{
	CDocument* pdoc = m_pwnd->GetDocument();

	VIEW_SETTINGS& view = pdoc->GetViewSettings();
	CGLView* glview = m_pwnd->GetGLView();

	CGLCamera& cam = glview->GetCamera();

	view.m_col1 = toGLColor(ui->m_bg->m_bg1);
	view.m_col2 = toGLColor(ui->m_bg->m_bg2);
	view.m_fgcol = toGLColor(ui->m_bg->m_fg);
	view.m_nbgstyle = ui->m_bg->m_nstyle;

	view.m_mcol = toGLColor(ui->m_display->m_meshColor);
	view.m_node_size = (float) ui->m_display->m_nodeSize;
	view.m_line_size = (float) ui->m_display->m_lineSize;
	view.m_bnorm = ui->m_display->m_bnormal;
	view.m_scaleNormals = ui->m_display->m_scaleNormal;
    view.m_nconv = ui->m_display->m_nconv;
	view.m_transparencyMode = ui->m_display->m_ntrans;
	view.m_objectColor = ui->m_display->m_nobjcol;

	view.m_brigid = ui->m_physics->m_showRigidJoints;
	view.m_bwall = ui->m_physics->m_showRigidWalls;
	view.m_bfiber = ui->m_physics->m_showFibers;
	view.m_fiber_scale = ui->m_physics->m_fiberScale;
	view.m_blma = ui->m_physics->m_showMatAxes;
	view.m_showHiddenFibers = ui->m_physics->m_showHiddenFibers;

	view.m_apply = (ui->m_ui->m_apply ? 1 : 0);
	view.m_clearUndoOnSave = ui->m_ui->m_bcmd;
	
	view.m_bconn = ui->m_select->m_bconnect;
	view.m_ntagInfo = ui->m_select->m_ntagInfo;
	view.m_bcullSel = ui->m_select->m_backface;
	view.m_bext = ui->m_select->m_binterior;
	view.m_bpart = ui->m_select->m_bpart;

	view.m_bLighting = ui->m_light->m_blight;
	view.m_diffuse   = ui->m_light->m_diffuse;
	view.m_ambient   = ui->m_light->m_ambient;
	view.m_bShadows  = ui->m_light->m_bshadow;
	view.m_shadow_intensity = ui->m_light->m_shadow;
	if (glview) glview->SetLightPosition(ui->m_light->m_pos);

	cam.SetCameraBias(ui->m_cam->m_bias);
	cam.SetCameraSpeed(ui->m_cam->m_speed);

	m_pwnd->setCurrentTheme(ui->m_ui->m_theme);

	m_pwnd->RedrawGL();
}

void CDlgSettings::accept()
{
	apply();
	QDialog::accept();
}

void CDlgSettings::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}
