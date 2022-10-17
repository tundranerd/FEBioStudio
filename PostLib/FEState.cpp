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
#include "FEState.h"
#include "FEPostMesh.h"
#include "FEPostModel.h"
#include "FEMeshData_T.h"

using namespace Post;
using namespace std;

ObjectData::ObjectData()
{
	data = nullptr;
	nsize = 0;
}

ObjectData::~ObjectData()
{
	delete [] data;
	nsize = 0;
}

void ObjectData::append(int n)
{
	off.push_back(nsize);
	if (data == nullptr)
	{
		data = new float[n];
		nsize = n;
	}
	else
	{
		int newSize = nsize + n;
		float* tmp = new float[newSize];
		memcpy(tmp, data, sizeof(float)*nsize);
		delete data;
		data = tmp;
		nsize = newSize;
	}
}

void ObjectData::push_back(float f)
{
	int n = nsize;
	append(1);
	data[n] = f;
}

void ObjectData::push_back(vec3f f)
{
	int n = nsize;
	append(3);
	data[n  ] = f.x;
	data[n+1] = f.y;
	data[n+2] = f.z;
}

FERefState::FERefState(FEPostModel* fem)
{

}

//-----------------------------------------------------------------------------
// Constructor
FEState::FEState(float time, FEPostModel* fem, Post::FEPostMesh* pmesh) : m_fem(fem), m_mesh(pmesh)
{
	m_id = -1;
	m_ref = nullptr; // will be set by model

	Post::FEPostMesh& mesh = *m_mesh;

	int nodes = mesh.Nodes();
	int edges = mesh.Edges();
	int elems = mesh.Elements();
	int faces = mesh.Faces();

	// allocate storage
	m_NODE.resize(nodes);
	m_EDGE.resize(edges);
	m_ELEM.resize(elems);
	m_FACE.resize(faces);

	// allocate element data
	for (int i=0; i<elems; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		m_ElemData.append(ne);
	}

	// allocate face data
	for (int i=0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		m_FaceData.append(nf);
	}

	// initialize data
	for (int i=0; i<nodes; ++i) m_NODE[i].m_rt = to_vec3f(mesh.Node(i).r);
	for (int i=0; i<elems; ++i)
	{
		m_ELEM[i].m_state = StatusFlags::VISIBLE;
		m_ELEM[i].m_h[0] = 0.f;
		m_ELEM[i].m_h[1] = 0.f;
		m_ELEM[i].m_h[2] = 0.f;
		m_ELEM[i].m_h[3] = 0.f;
	}

	int ptObjs = fem->PointObjects();
	m_objPt.resize(ptObjs);
	for (int i = 0; i < ptObjs; ++i)
	{
		OBJ_POINT_DATA& di = m_objPt[i];
		Post::FEPostModel::PointObject& po = *fem->GetPointObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_rt = po.m_rt;

		int ndata = po.m_data.size();
		di.data = new ObjectData;
		for (int j = 0; j < ndata; ++j)
		{
			Post::PlotObjectData& dj = *po.m_data[j];

			switch (dj.Type())
			{
			case DATA_FLOAT: di.data->push_back(0.f); break;
			case DATA_VEC3F: di.data->push_back(vec3f(0.f, 0.f, 0.f)); break;
			default:
				assert(false);
			}
		}
	}

	int lnObjs = fem->LineObjects();
	m_objLn.resize(lnObjs);
	for (int i = 0; i < lnObjs; ++i)
	{
		OBJ_LINE_DATA& di = m_objLn[i];
		Post::FEPostModel::LineObject& po = *fem->GetLineObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_r1 = po.m_r1;
		di.m_r2 = po.m_r2;

		int ndata = po.m_data.size();
		di.data = new ObjectData;
		for (int j = 0; j < ndata; ++j)
		{
			Post::PlotObjectData& dj = *po.m_data[j];

			switch (dj.Type())
			{
			case DATA_FLOAT: di.data->push_back(0.f); break;
			case DATA_VEC3F: di.data->push_back(vec3f(0.f, 0.f, 0.f)); break;
			default:
				assert(false);
			}
		}
	}

	m_time = time;
	m_nField = -1;
	m_status = 0;

	// get the data manager
	FEDataManager* pdm = fem->GetDataManager();

	// Nodal data
	int N = pdm->DataFields();
	FEDataFieldPtr it = pdm->FirstDataField();
	for (int i=0; i<N; ++i, ++it)
	{
		ModelDataField& d = *(*it);
		m_Data.push_back(d.CreateData(this));
	}
}

//-----------------------------------------------------------------------------
// helper function for copying data
template <class T> void copyData(Post::FEMeshData* dest, Post::FEMeshData* src)
{ 
	T* pd = dynamic_cast<T*>(dest);
	T* ps = dynamic_cast<T*>(src);
	if (pd && ps) pd->copy(*ps); 
	else 
	{
		// We can get here for implicit data fields (e.g. LagrangeStrain). 
		// which is probably okay.
//		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEState::SetID(int n)
{
	m_id = n;
}

//-----------------------------------------------------------------------------
int FEState::GetID() const 
{ 
	assert(m_id != -1);
	return m_id; 
}

//-----------------------------------------------------------------------------
// Constructor
FEState::FEState(float time, FEPostModel* pfem, FEState* pstate) : m_fem(pfem)
{
	m_id = -1;
	m_time = time;
	m_nField = -1;
	m_status = 0;
	m_mesh = pstate->m_mesh;

	RebuildData();

	// get the data manager
	FEDataManager* pdm = pfem->GetDataManager();

	// Nodal data
	int N = pdm->DataFields();
	FEDataFieldPtr pn = pdm->FirstDataField();
	for (int i = 0; i < N; ++i, ++pn)
	{
		m_Data.push_back((*pn)->CreateData(this));
	}

	// copy data
	pn = pdm->FirstDataField();
	for (int i = 0; i < N; ++i, ++pn)
	{
		ModelDataField& d = *(*pn);
		FEMeshData& md = m_Data[i];
		if (d.DataClass() == CLASS_NODE)
		{
			switch (md.GetType())
			{
			case DATA_FLOAT: copyData< Post::FENodeData<FEDataTypeTraits<DATA_FLOAT  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3F: copyData< Post::FENodeData<FEDataTypeTraits<DATA_VEC3F  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3F: copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3F  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3D: copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3D  >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FS: copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3FS >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FD: copyData< Post::FENodeData<FEDataTypeTraits<DATA_MAT3FD >::dataType> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4FS: copyData< Post::FENodeData<FEDataTypeTraits<DATA_TENS4FS>::dataType> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
		else if (d.DataClass() == CLASS_FACE)
		{
			switch (md.GetType())
			{
			case DATA_FLOAT: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_FLOAT  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3F: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_VEC3F  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3F: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3F  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3D: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3D  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FS: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3FS >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FD: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_MAT3FD >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4FS: copyData< Post::FEFaceData<FEDataTypeTraits<DATA_TENS4FS>::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
		else if (d.DataClass() == CLASS_ELEM)
		{
			switch (md.GetType())
			{
			case DATA_FLOAT: copyData< Post::FEElementData<FEDataTypeTraits<DATA_FLOAT  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_VEC3F: copyData< Post::FEElementData<FEDataTypeTraits<DATA_VEC3F  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3F: copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3F  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3D: copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3D  >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FS: copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3FS >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_MAT3FD: copyData< Post::FEElementData<FEDataTypeTraits<DATA_MAT3FD >::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			case DATA_TENS4FS: copyData< Post::FEElementData<FEDataTypeTraits<DATA_TENS4FS>::dataType, DATA_ITEM> >(&md, &pstate->m_Data[i]); break;
			default:
				assert(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEState::RebuildData()
{
	FEPostMesh& mesh = *m_mesh;
	FEPostModel& fem = *m_fem;

	int nodes = mesh.Nodes();
	int edges = mesh.Edges();
	int elems = mesh.Elements();
	int faces = mesh.Faces();

	// allocate storage
	m_NODE.resize(nodes);
	m_EDGE.resize(edges);
	m_ELEM.resize(elems);
	m_FACE.resize(faces);

	// allocate element data
	m_ElemData.clear();
	for (int i = 0; i < elems; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		m_ElemData.append(ne);

		m_ELEM[i].m_state = StatusFlags::VISIBLE;
	}

	// allocate face data
	m_FaceData.clear();
	for (int i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		m_FaceData.append(nf);
	}

	// initialize data
	for (int i = 0; i < nodes; ++i) m_NODE[i].m_rt = to_vec3f(mesh.Node(i).r);
	for (int i = 0; i < elems; ++i)
	{
		m_ELEM[i].m_h[0] = 0.f;
		m_ELEM[i].m_h[1] = 0.f;
		m_ELEM[i].m_h[2] = 0.f;
		m_ELEM[i].m_h[3] = 0.f;
	}

	int ptObjs = fem.PointObjects();
	m_objPt.resize(ptObjs);
	for (int i = 0; i < ptObjs; ++i)
	{
		OBJ_POINT_DATA& di = m_objPt[i];
		Post::FEPostModel::PointObject& po = *fem.GetPointObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_rt = po.m_rt;
	}

	int lnObjs = fem.LineObjects();
	m_objLn.resize(lnObjs);
	for (int i = 0; i < lnObjs; ++i)
	{
		OBJ_LINE_DATA& di = m_objLn[i];
		Post::FEPostModel::LineObject& po = *fem.GetLineObject(i);

		di.pos = po.m_pos;
		di.rot = po.m_rot;

		di.m_r1 = po.m_r1;
		di.m_r2 = po.m_r2;
	}
}

//-----------------------------------------------------------------------------
OBJECT_DATA& FEState::GetObjectData(int n)
{
	if (n < m_objPt.size()) return m_objPt[n];
	else return m_objLn[n - m_objPt.size()];
}

//-----------------------------------------------------------------------------
class OctreeBox
{
public:
	OctreeBox(BOX box, int levels) : m_box(box), m_level(levels)
	{
		if (levels == 0)
		{
			for (int i = 0; i < 8; ++i) m_child[i] = nullptr;
			return;
		}

		double R = box.Radius();

		double x0 = box.x0, x1 = box.x1;
		double y0 = box.y0, y1 = box.y1;
		double z0 = box.z0, z1 = box.z1;
		int n = 0;
		for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
		for (int k = 0; k < 2; ++k)
		{
			double xa = x0 + i * (x1 - x0) * 0.5;
			double ya = y0 + j * (y1 - y0) * 0.5;
			double za = z0 + k * (z1 - z0) * 0.5;
			double xb = x0 + (i+1.0) * (x1 - x0) * 0.5;
			double yb = y0 + (j+1.0) * (y1 - y0) * 0.5;
			double zb = z0 + (k+1.0) * (z1 - z0) * 0.5;
			BOX boxi(xa, ya, za, xb, yb, zb);
			boxi.Inflate(R * 1e-7);
			m_child[n++] = new OctreeBox(boxi, levels - 1);
		}
	}
	~OctreeBox() { for (int i = 0; i < 8; ++i) delete m_child[i]; }

	int addNode(vector<vec3d>& points, const vec3d& r)
	{
		const double eps = 1e-12;
		if (m_level == 0)
		{
			if (m_box.IsInside(r))
			{
				for (int i = 0; i < m_nodes.size(); ++i)
				{
					vec3d& ri = points[m_nodes[i]];
					if ((ri - r).SqrLength() <= eps)
					{
						// node is already in list
						return m_nodes[i];
					}
				}

				// if we get here, the node is in this box, 
				// but not in the points array yet, so add it
				points.push_back(r);
				m_nodes.push_back(points.size() - 1);
				return points.size() - 1;
			}
			else return -1;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				int n = m_child[i]->addNode(points, r);
				if (n >= 0) return n;
			}
			return -1;
		}
	}

private:
	int			m_level;
	BOX			m_box;
	OctreeBox*	m_child[8];
	vector<int>	m_nodes;
};
