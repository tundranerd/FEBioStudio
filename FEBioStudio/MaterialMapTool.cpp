#include "stdafx.h"
#include "MaterialMapTool.h"
#include <LSDYNA/FELSDYNAimport.h>
#include "Document.h"
#include <PreViewLib/ColorMap.h>
#include <PreViewLib/FEMultiMaterial.h>

CMaterialMapTool::CMaterialMapTool() : CBasicTool("Material Map", HAS_APPLY_BUTTON)
{
	m_nmat = 5;

	QStringList flt;
	flt << "LSDYNA keyword files (*.k)";

	addResourceProperty(&m_file, "E")->setEnumValues(flt);
	addIntProperty(&m_nmat, "Materials");
}

bool CMaterialMapTool::OnApply()
{
	if ((m_nmat < 1) || (m_nmat > 100))
	{
		SetErrorString("Materials must be a number between 1 and 100.");
		return false;
	}

	CDocument* pdoc = GetDocument();

	std::string sfile = m_file.toStdString();

	FELSDYNAimport reader;
	if (!pdoc->ImportGeometry(&reader, sfile.c_str()))
	{
		SetErrorString(QString::fromStdString(reader.GetErrorMessage()));
		return false;
	}
	else
	{
		FEModel& fem = *pdoc->GetFEModel();
		GObject* po = fem.GetModel().Object(0);
		FEMesh& mesh = *po->GetFEMesh();
		int N = mesh.Nodes();
		int NE = mesh.Elements();

		// find the E-range
		double Emin, Emax;
		Emin = Emax = reader.NodeData(0, 0);
		for (int i = 1; i<N; ++i)
		{
			double E = reader.NodeData(i, 0);
			if (E < Emin) Emin = E;
			if (E > Emax) Emax = E;
		}
		if (Emin == Emax) Emax += 1.0;

		// find the v-range
		double vmin, vmax;
		vmin = vmax = reader.NodeData(0, 1);
		for (int i = 1; i<N; ++i)
		{
			double v = reader.NodeData(i, 1);
			if (v < vmin) vmin = v;
			if (v > vmax) vmax = v;
		}
		if (vmin == vmax) vmax += 1.0;

		if (m_nmat > 1)
		{
			// set the element material ID's
			for (int i = 0; i<NE; ++i)
			{
				FEElement& e = mesh.Element(i);
				int ne = e.Nodes();
				double E = 0;
				for (int j = 0; j<ne; ++j) E += reader.NodeData(e.m_node[j], 0);
				E /= (double)ne;

				double v = 0;
				for (int j = 0; j<ne; ++j) v += reader.NodeData(e.m_node[j], 1);
				v /= (double)ne;

				int n0 = (int)(m_nmat*(E - Emin) / (Emax - Emin));
				int n1 = (int)(m_nmat*(v - vmin) / (vmax - vmin));

				if (n0 >= m_nmat) n0 = m_nmat - 1;
				if (n0 < 0) n0 = 0;

				if (n1 >= m_nmat) n1 = m_nmat - 1;
				if (n1 < 0) n1 = 0;

				e.m_ntag = n0*m_nmat + n1;
			}

			// rebuild the object
			po->Update();

			// create the materials
			CColorMap col;
			col.jet();
			for (int i = 0; i<m_nmat; ++i)
			{
				double f = (double)i / (m_nmat - 1.f);
				for (int j = 0; j<m_nmat; ++j)
				{
					double g = (double)j / (m_nmat - 1.f);

					FENeoHookean* pm = new FENeoHookean;
					pm->SetFloatValue(FENeoHookean::MP_E, Emin + f*(Emax - Emin));
					pm->SetFloatValue(FENeoHookean::MP_v, vmin + g*(vmax - vmin));

					double h = (double)(i*m_nmat + j) / (m_nmat*m_nmat - 1.f);
					GMaterial* pgm = new GMaterial(pm);
					pgm->Diffuse(col.map(h*255.f));
					pgm->Ambient(col.map(h*255.f));
					fem.AddMaterial(pgm);
				}
			}

			int NP = po->Parts();
			vector<int> MP; MP.assign(NP, 0);
			for (int i = 0; i<NE; ++i)
			{
				FEElement& e = mesh.Element(i);
				MP[e.m_gid] = e.m_ntag;
			}

			for (int i = 0; i<po->Parts(); ++i)
			{
				po->AssignMaterial(po->Part(i)->GetID(), fem.GetMaterial(MP[i])->GetID());
			}
		}
	}

	return true;
}
