#include "stdafx.h"
#include "GModifiedObject.h"
#include "FEModel.h"

//-----------------------------------------------------------------------------
GModifiedObject::GModifiedObject(GObject* po) : GObject(GMODIFIED_OBJECT)
{
	m_po = 0;
	m_pStack = new GModifierStack(this);
	
	if (po) 
	{
		SetChildObject(po);
		Update();
	}
}

//-----------------------------------------------------------------------------
GModifiedObject::~GModifiedObject(void)
{
	// We have to delete the reference object ourselves
	delete m_po;

	// TODO: I think I can get rid of the ownership stuff
	if (m_pStack->GetOwner() == this) delete m_pStack;
	m_pStack = 0;
}

//-----------------------------------------------------------------------------
void GModifiedObject::SetChildObject(GObject *po, bool bclone)
{
	assert(m_po == 0);

	// set the reference object
	m_po = po;

	// copy ID's
	m_gid = po->GetID();
	m_lid = po->GetLocalID();

	// only copy this data if we are cloning the child-object
	// We usually clone, except when we load the object from file
	if (bclone)
	{
		// copy name
		SetName(po->GetName());

		// copy object transform data
		CopyTransform(po);

		// copy object appearance data
		SetColor(po->GetColor());

		// clone child data
		CloneChild();
	}

	Update(false);
}

//-----------------------------------------------------------------------------
void GModifiedObject::CloneChild()
{
	int i, N;

	// clear data
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	m_Part.clear();

	// copy object's nodal data
	N = m_po->Nodes(); for (i = 0; i<N; ++i) { GNode* node = new GNode(this); *node = *m_po->Node(i); m_Node.push_back(node); }
	N = m_po->Edges(); for (i = 0; i<N; ++i) { GEdge* edge = new GEdge(this); *edge = *m_po->Edge(i); m_Edge.push_back(edge); }
	N = m_po->Parts(); for (i = 0; i<N; ++i) { GPart* part = new GPart(this); *part = *m_po->Part(i); m_Part.push_back(part); }
	N = m_po->Faces(); for (i = 0; i<N; ++i) { GFace* face = new GFace(this); *face = *m_po->Face(i); m_Face.push_back(face); }
}

//-----------------------------------------------------------------------------
void GModifiedObject::AddModifier(GModifier* pmod)
{
	m_pStack->Add(pmod);
}

//-----------------------------------------------------------------------------
void GModifiedObject::DeleteModifier(GModifier* pmod)
{
	assert(m_pStack->Size()>0);
	m_pStack->Remove(pmod);
}

//-----------------------------------------------------------------------------
FEMesh* GModifiedObject::BuildMesh()
{
	delete m_pmesh;
	m_pmesh = 0;

	// ask the ref object to build a mesh
	FEMesh* pm = m_po->BuildMesh();
	if (pm)
	{
		m_pmesh = new FEMesh(*pm);
		m_pmesh->SetGObject(this);
	}
	
	// apply modifiers to FEMesh
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i)
	{
		GModifier* pmod = m_pStack->Modifier(i);
		FEMesh* pm = pmod->BuildFEMesh(this);
		if (pm) SetFEMesh(pm);
	}

	// Make sure the normals and the bounding box are up to date.
	if (m_pmesh)
	{
		m_pmesh->UpdateNormals();
		m_pmesh->UpdateBox();
	}
	return m_pmesh;
}

//-----------------------------------------------------------------------------
void GModifiedObject::BuildGMesh()
{
	delete m_pGMesh;
	m_po->BuildGMesh();
	GLMesh* pm = m_po->GetRenderMesh();
	m_pGMesh = new GLMesh(*pm);
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i)
	{
		m_pStack->Modifier(i)->BuildGMesh(this);
		m_pGMesh->UpdateBoundingBox();
	}
	m_pGMesh->UpdateNormals();
}

//-----------------------------------------------------------------------------
bool GModifiedObject::Update(bool b)
{
	if (m_po->Update(b) == false) return false;

	// restore the original object
	CloneChild();

	// apply modifiers to GObject
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i) m_pStack->Modifier(i)->Apply(this);

	// rebuild the GMesh
	BuildGMesh();

	// if we have an FE mesh, we should rebuild that one as well
	if (m_po->GetFEMesh()) BuildMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GModifiedObject::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, Transform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, Transform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, Transform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parts
	ar.BeginChunk(CID_OBJ_PART_SECTION);
	{
		for (int i=0; i<Parts(); ++i)
		{
			ar.BeginChunk(CID_OBJ_PART);
			{
				GPart& p = *Part(i);
				int nid = p.GetID();
				int mid = p.GetMaterialID();
				ar.WriteChunk(CID_OBJ_PART_ID, nid);
				ar.WriteChunk(CID_OBJ_PART_MAT, mid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the surfaces
	ar.BeginChunk(CID_OBJ_FACE_SECTION);
	{
		for (int i=0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
				ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
				ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_SECTION);
	{
		for (int i=0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_SECTION);
		{
			for (int i=0; i<Nodes(); ++i)
			{	
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the child object
	ar.BeginChunk(CID_OBJ_GOBJECTS);
	{
		int ntype = m_po->GetType();
		ar.BeginChunk(ntype);
		{
			m_po->Save(ar);
		}
		ar.EndChunk();
	}
	ar.EndChunk();

	// save the modifiers
	if (m_pStack->Size() > 0)
	{
		ar.BeginChunk(CID_MODIFIER_STACK);
		{
			m_pStack->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
extern GObject* BuildObject(int);

//-----------------------------------------------------------------------------
void GModifiedObject::Load(IArchive &ar)
{
	TRACE("GModifiedObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;
	GObject* po = 0;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		// object name
		case CID_OBJ_NAME: 
			{
				char sz[256];
				ar.read(sz);
				SetName(sz);
			}
			break;
		// header
		case CID_OBJ_HEADER:
			{
				vec3d pos, scl;
				quatd rot;
				GLColor col;
				while (IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					int oid;
					switch (nid)
					{
					case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
					case CID_OBJ_POS: ar.read(pos); break;
					case CID_OBJ_ROT: ar.read(rot); break;
					case CID_OBJ_SCALE: ar.read(scl); break;
					case CID_OBJ_COLOR: ar.read(col); break;
					case CID_OBJ_PARTS: ar.read(nparts); break;
					case CID_OBJ_FACES: ar.read(nfaces); break;
					case CID_OBJ_EDGES: ar.read(nedges); break;
					case CID_OBJ_NODES: ar.read(nnodes); break;
					}
					ar.CloseChunk();
				}

				SetColor(col);

				GTransform& transform = Transform();
				transform.SetPosition(pos);
				transform.SetRotation(rot);
				transform.SetScale(scl);
			}
			break;
		// object parts
		case CID_OBJ_PART_SECTION:
			{
				assert(nparts > 0);
				m_Part.reserve(nparts);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GModifiedObject::Load)");

					GPart* p = new GPart(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID : ar.read(nid); p->SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p->SetLocalID(n++);

					m_Part.push_back(p);
				}
				assert((int) m_Part.size() == nparts);
			}
			break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
			{
				assert(nfaces > 0);
				m_Face.reserve(nfaces);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GModifiedObject::Load)");

					GFace* f = new GFace(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID  : ar.read(nid); f->SetID(nid); break;
						case CID_OBJ_FACE_PID0: ar.read(nid); f->m_nPID[0] = nid; break;
						case CID_OBJ_FACE_PID1: ar.read(nid); f->m_nPID[1] = nid; break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f->SetLocalID(n++);

					m_Face.push_back(f);
				}
				assert((int) m_Face.size() == nfaces);
			}
			break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
			{
				m_Edge.clear();
				if (nedges > 0) m_Edge.reserve(nedges);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GModifiedObject::Load)");

					GEdge* e = new GEdge(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e->SetLocalID(n++);

					m_Edge.push_back(e);
				}
				assert((int) m_Edge.size() == nedges);
			}
			break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
			{
				m_Node.clear();
				if (nnodes > 0)
				{
					m_Node.reserve(nnodes);
					int m = 0;
					while (IO_OK == ar.OpenChunk())
					{
						if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GModifiedObject::Load)");

						GNode* n = new GNode(this);
						while (IO_OK == ar.OpenChunk())
						{
							int nid;
							switch (ar.GetChunkID())
							{
							case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
							case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
							}
							ar.CloseChunk();
						}
						ar.CloseChunk();

						n->SetLocalID(m++);

						m_Node.push_back(n);
					}
					assert((int) m_Node.size() == nnodes);
				}
			}
			break;
		// child object
		case CID_OBJ_GOBJECTS:
			{
				while (IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();
					assert(m_po == 0);
					po = BuildObject(ntype);
					if (po == 0) throw ReadError("error parsing CID_OBJ_GOBJECTS (GModifiedObject::Load)");
					po->Load(ar);
					ar.CloseChunk();
				}
			}
			break;
		// modifier stack
		case CID_MODIFIER_STACK:
			{
				m_pStack->Load(ar);
			}
			break;
		}
		ar.CloseChunk();
	}
	assert(po);
	SetChildObject(po, false);
}

//-----------------------------------------------------------------------------
FEMesher* GModifiedObject::GetMesher()
{
	if (m_pMesher == 0) return m_po->GetMesher();
	return m_pMesher;
}

//-----------------------------------------------------------------------------
GObject* GModifiedObject::Clone()
{
	// first, we clone the child object
	GObject* pc = m_po->Clone();
	if (pc == 0) return 0;

	// next, we create a new modifier object
	GModifiedObject* po = new GModifiedObject(pc);

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	// copy the modifier stack
	po->m_pStack->Copy(m_pStack);

	// update the new object
	po->Update();

	return po;
}
