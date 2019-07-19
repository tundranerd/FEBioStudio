#include "stdafx.h"
#include "FoamGeneratorTool.h"
#include "Document.h"
#include <GeomLib/GMeshObject.h>
#include <QMessageBox>

CFoamGeneratorTool::CFoamGeneratorTool() : CBasicTool("Foam generator", HAS_APPLY_BUTTON)
{
	addIntProperty(&m_foam.m_nx, "nx");
	addIntProperty(&m_foam.m_ny, "ny");
	addIntProperty(&m_foam.m_nz, "nz");
	addIntProperty(&m_foam.m_nseed, "seeds");
	addIntProperty(&m_foam.m_nsmooth, "smooth iters.");
	addDoubleProperty(&m_foam.m_ref, "ref");
}

bool CFoamGeneratorTool::OnApply()
{
	static int n = 1;

	// get the document
	CDocument* doc = GetDocument();
	FEModel* ps = doc->GetFEModel();

	// create the foam mesh
	FEMesh* pm = m_foam.Create();
	if (pm)
	{
		char szname[256] = { 0 };

		// get the active object
		GMeshObject* pa = dynamic_cast<GMeshObject*>(doc->GetActiveObject());

		if (pa) strcpy(szname, pa->GetName().c_str());
		else
		{
			sprintf(szname, "Foam%02d", n++);
		}

		// if this is the last selected object, delete it so we can replace it
		if (pa && (pa == m_pfo)) doc->DoCommand(new CCmdDeleteObject(pa));

		// Create a new GMeshObject
		GMeshObject* po = new GMeshObject(pm);
		po->SetName(szname);
		m_pfo = po;

		// add the new object to the model
		doc->DoCommand(new CCmdAddAndSelectObject(po));
	}
	else 
	{
		SetErrorString("Mesh generation has failed.");
		return false;
	}

	return true;
}
