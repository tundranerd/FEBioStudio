/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "GMaterial.h"
#include <FEMLib/FEUserMaterial.h>
#include <FEMLib/FEMaterial.h>
#include <sstream>

using std::stringstream;

int GMaterial::m_nref = 1;

GLColor col[GMaterial::MAX_COLORS] = {
	GLColor(240, 164, 96),
	GLColor(240, 240, 0),
	GLColor(240, 0, 240),
	GLColor(0, 240, 240),
	GLColor(240, 180, 0),
	GLColor(240, 0, 180),
	GLColor(180, 240, 0),
	GLColor(0, 240, 180),
	GLColor(180, 0, 240),
	GLColor(0, 180, 240),
	GLColor(0, 180, 0),
	GLColor(0, 0, 180),
	GLColor(180, 180, 0),
	GLColor(0, 180, 180),
	GLColor(180, 0, 180),
	GLColor(120, 0, 240)
};

GMaterial::GMaterial(FEMaterial* pm)
{
	m_pm = pm;
	if (m_pm) m_pm->SetOwner(this);
	m_ntag = 0;

	// set unique material ID
	m_nID = m_nref++;

	// give a default color
	m_diffuse = GLColor(255,255,255);
	m_ambient = GLColor(128,128,128);
	m_specular = GLColor(0,0,0);
	m_emission = GLColor(0,0,0);
	m_shininess = 1.f;

	// set default rendering style
	m_nrender = 0; // solid rendering

	// set a default name for this material
	stringstream ss;
	ss << "Material" << m_nID;
	SetName(ss.str());

	AmbientDiffuse(col[(m_nID-1) % 16]);
}

GMaterial::~GMaterial(void)
{
	delete m_pm;
}

void GMaterial::SetMaterialProperties(FEMaterial* pm) 
{ 
	delete m_pm; 
	m_pm = pm; 
	m_pm->SetOwner(this);
}

FEMaterial* GMaterial::GetMaterialProperties()
{ 
	return m_pm; 
}

GMaterial* GMaterial::Clone()
{
	FEMaterial* pmCopy = 0;
	if (m_pm) pmCopy = m_pm->Clone();
	return new GMaterial(pmCopy);
}

const char* GMaterial::GetFullName()
{
	static char sz[256];

	if (m_pm->Type() == FE_USER_MATERIAL)
	{
		FEUserMaterial* pm = dynamic_cast<FEUserMaterial*>(m_pm);
		sprintf(sz, "%s (%s)", GetName().c_str(), pm->GetTypeString());
	}
	else
	{
		FEMaterialFactory* pMF = FEMaterialFactory::GetInstance();

		FEMatDescriptor* pmd = pMF->Find(m_pm->Type());
		assert(pmd);
		sprintf(sz, "%s (%s)", GetName().c_str(), pmd->GetTypeString());
	}

	return sz;
}

void GMaterial::Save(OArchive &ar)
{
	ar.WriteChunk(CID_MAT_ID, m_nID);
	ar.WriteChunk(CID_MAT_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_MAT_DIFFUSE, m_diffuse);
	ar.WriteChunk(CID_MAT_AMBIENT, m_ambient);
	ar.WriteChunk(CID_MAT_SPECULAR, m_specular);
	ar.WriteChunk(CID_MAT_EMISSION, m_emission);
	ar.WriteChunk(CID_MAT_SHININESS, m_shininess);
	ar.WriteChunk(CID_MAT_RENDER, m_nrender);

	if (m_pm)
	{
		ar.BeginChunk(CID_MAT_PARAMS);
		{
			m_pm->Save(ar);
		}
		ar.EndChunk();
	}
}

void GMaterial::Load(IArchive &ar)
{
	TRACE("GMaterial::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MAT_ID: { int mid; ar.read(mid); SetID(mid); } break;
		case CID_MAT_NAME: { string name; ar.read(name); SetName(name); break; }
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_MAT_DIFFUSE: ar.read(m_diffuse); break;
		case CID_MAT_AMBIENT: ar.read(m_ambient); break;
		case CID_MAT_SPECULAR: ar.read(m_specular); break;
		case CID_MAT_EMISSION: ar.read(m_emission); break;
		case CID_MAT_SHININESS: ar.read(m_shininess); break;
		case CID_MAT_RENDER: ar.read(m_nrender); break;
		case CID_MAT_PARAMS: assert(m_pm); m_pm->Load(ar); break;
		}
		ar.CloseChunk();
	}
}
