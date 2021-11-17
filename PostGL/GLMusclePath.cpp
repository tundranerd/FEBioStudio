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
#include "GLMusclePath.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/triangulate.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

class GLMusclePath::PathData
{
public:
	enum { PathLength, MomentArm };

public:
	PathData() : m_hull(nullptr) {}
	~PathData()
	{
		if (m_hull) delete m_hull;
	}

public:
	std::vector<vec3d>	m_points;		// points defining the path
	::FEMesh*			m_hull;			// the convex hull

	vec3d	m_ro;		// position of origin

	double		m_data[2];	// 0 = length, 1 = moment arm

private:
	PathData(const PathData& path) {}
};

REGISTER_CLASS(GLMusclePath, CLASS_PLOT, "muscle-path", 0);

static int n = 1;
GLMusclePath::GLMusclePath()
{
	SetTypeString("muscle-path");

	AddIntParam(0, "point0", "start point");
	AddIntParam(0, "point1", "end point");
	AddVecParam(vec3d(0, 0, 0), "center", "Center of rotation");
	AddChoiceParam(0, "method", "Shortest path method")->SetEnumNames("Straight line\0 3-point trace\0Shortest path 2D\0Shortest path\0Spring\0");
	AddDoubleParam(5.0, "size");
	AddColorParam(GLColor(255, 0, 0), "color");
	AddBoolParam(true, "show_convex_hull", "Draw convex hull");

	std::stringstream ss;
	ss << "MusclePath" << n++;
	SetName(ss.str());

	m_closestFace = -1;

	// we need the active face selection 
/*/	if (fem)
	{
		FEPostMesh& mesh = *fem->GetActiveMesh();
		mesh.TagAllNodes(0);
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& f = mesh.Face(i);
			if (f.IsSelected())
			{
				for (int j = 0; j < f.Nodes(); ++j) mesh.Node(f.n[j]).m_ntag = 1;
			}
		}

		// get all the tagged nodes
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			if (mesh.Node(i).m_ntag == 1) m_selNodes.push_back(i);
		}
	}
*/
}

GLMusclePath::~GLMusclePath()
{
	ClearPaths();
}

void GLMusclePath::ClearPaths()
{
	for (int i = 0; i < m_path.size(); ++i)
	{
		delete m_path[i];
	}
	m_path.clear();
}

void GLMusclePath::Render(CGLContext& rc)
{
	if (m_path.empty()) return;

	CGLModel* glm = GetModel();

	int nstate = glm->CurrentTimeIndex();
	if ((nstate < 0) || (nstate >= m_path.size())) return;

	PathData* path = m_path[nstate];
	if (path == nullptr) return;

	double R = GetFloatValue(SIZE);
	GLColor c = GetColorValue(COLOR);

	// draw the path
	int N = (int) path->m_points.size();
	if (N > 1)
	{
		vec3d r0 = path->m_points[0];
		vec3d r1 = path->m_points[N - 1];

		// draw the muscle path
		glColor3ub(c.r, c.g, c.b);
		glx::drawSmoothPath(path->m_points, R);

		// draw the end points
		glx::drawSphere(r0, 1.5 * R);

		glColor3ub(255, 0, 255);
		glx::drawSphere(r1, 1.5 * R);

		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; ++i)
		{
			vec3d r0 = path->m_points[i];
			vec3d r1 = path->m_points[i + 1];
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
		glEnd();
		glPopAttrib();

		for (int i = 0; i < N; ++i)
		{
			vec3d r0 = path->m_points[i];
			glx::drawSphere(r0, 1.5 * R);
		}
	}

	// draw the rotation center
	vec3d o = path->m_ro;
	glColor3ub(255, 255, 0);
	glx::drawSphere(o, R);

	// draw the convex hull for the path
	bool bdrawDebug = GetBoolValue(DRAW_DEBUG);
	if (path->m_hull && bdrawDebug)
	{
		GLMeshRender gl;
		glColor3ub(255, 64, 164);
		gl.RenderMeshLines(path->m_hull);
	}
}

void GLMusclePath::Update()
{
	Update(GetModel()->CurrentTimeIndex(), 0.f, false);
}

void GLMusclePath::Update(int ntime, float dt, bool breset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	if (breset)
	{
		m_closestFace = -1;

		// clear current path data
		ClearPaths();

		// allocate new path data
		m_path.assign(fem.GetStates(), nullptr);

		// update point selection
		FEMesh& mesh = *glm->GetActiveMesh();
		m_selNodes.clear();
//		for (int i = 0; i < mesh.Nodes(); ++i) m_selNodes.push_back(i);
	}

	if ((ntime < 0) || (ntime >= m_path.size())) return;

	// If we already calculated the path for this time step, we're done
	if (m_path[ntime] != nullptr) return;

	UpdatePath(ntime);
}

void GLMusclePath::UpdatePath(int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	int NN = mesh.Nodes();
	if ((n0 < 0) || (n0 >= NN)) return;
	if ((n1 < 0) || (n1 >= NN)) return;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	PathData* path = new PathData;

	// update the location of the reference configuration first
	// since the 3-point path method needs this
	path->m_ro = UpdateOrigin(ntime);

	// see which method we're going to use
	int method = GetIntValue(METHOD);
	bool b = false;
	switch (method)
	{
	case 0: b = UpdateStraighLine   (path, ntime); break;
	case 1: b = Update3PointPath    (path, ntime); break;
	case 2: b = UpdateShortestPath2D(path, ntime); break;
	case 3: b = UpdateShortestPath  (path, ntime); break;
	case 4: b = UpdateSpringPath    (path, ntime); break;
	}
	if (b == false) { delete path; path = nullptr; }

	// All is well, so assign the new path
	m_path[ntime] = path;

	// also update the path data
	UpdatePathData(ntime);
}

bool GLMusclePath::UpdateStraighLine(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	path->m_points.clear();
	path->m_points.push_back(r0);
	path->m_points.push_back(r1);

	return true;
}

bool GLMusclePath::Update3PointPath(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	// determine the three points on plane
	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);
	vec3d rc = path->m_ro;

	// determine plane normal
	vec3d N = (r1 - rc) ^ (r0 - rc);
	N.Normalize();

	// determine initial length
	double L0 = (r1 - r0).Length();

	// find all points of the mesh that intersect with this plane
	vector<vec3d> pt;
	pt.push_back(r0);
	vec3d ra = r0;
	const double dr = 0.1*L0;
	FEPostMesh& mesh = *glm->GetActiveMesh();
	do
	{
		// determine step direction
		vec3d t = r1 - ra; 
		t.Normalize();

		vec3d rb = ra + t * dr;

		// make sure we don't overstep
		double w = t * (r1 - ra);
		if (((rb - ra).Length() < 1e-12) || (w <= dr))
		{
			pt.push_back(r1);
			break;
		}

		vec3d n = rb - rc; n.Normalize();

		// find all intersections from origin to b
		vector<vec3d> q = FindAllIntersections(mesh, rc, n, false);
		if (q.empty() == false)
		{
			double Lmax = 0;
			int imax = -1;
			for (int i = 0; i < q.size(); ++i)
			{
				double Li = (q[i] - rc) * n;
				if ((imax == -1) || (Li > Lmax))
				{
					imax = i;
					Lmax = Li;
				}
			}
			assert(imax != -1);

			// add the farthest one
			rb = q[imax];
		}
		
		pt.push_back(rb);
		if (pt.size() > 20) break;
		ra = rb;
	} 
	while (true);

	// add the last point
	if ((ra - r1).Length() > 1e-7) pt.push_back(r1);

	path->m_points = pt;

	return true;
}

bool GLMusclePath::UpdateShortestPath2D(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	// determine the three points on plane
	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);
	vec3d rc = path->m_ro;

	// determine plane normal
	vec3d N = (r1 - rc) ^ (r0 - rc);
	N.Normalize();

	quatd Q(N, vec3d(0, 0, 1));

	// find all edge-intersections with this plane
	FEPostMesh& mesh = *glm->GetActiveMesh();
	vector<vec3d> pt;
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		vec3d ra = fem.NodePosition(edge.n[0], ntime);
		vec3d rb = fem.NodePosition(edge.n[1], ntime);

		double D = N * (rb - ra);
		if (D != 0.0)
		{
			double lam = N * (rc - ra) / D;
			if ((lam >= 0.0) && (lam <= 1.0))
			{
				vec3d q = ra + (rb - ra) * lam;

				// make sure the point q projects onto the r0-r1 line
				vec3d t = r1 - r0;
				double mu = (t * (q - r0)) / (t * t);

				if ((mu >= 0) && (mu <= 1.0))
				{
					vec3d q2 = Q * (q - rc);
					pt.push_back(q2);
				}
			}
		}
	}

	// calculate convex hull
	std::vector<vec3d> ch = convex_hull2d(pt);

	// find the original nodes on this hull
	int m0 = -1;
	int m1 = -1;
	double L0min, L1min;
	vec3d ra = Q * (r0 - rc);
	vec3d rb = Q * (r1 - rc);
	int n = ch.size();
	for (int i = 0; i < n; ++i)
	{
		vec3d ri = ch[i];
		double L0 = (ra - ri).SqrLength();
		double L1 = (rb - ri).SqrLength();

		if ((m0 == -1) || (L0 < L0min))
		{
			m0 = i;
			L0min = L0;
		}

		if ((m1 == -1) || (L1 < L1min))
		{
			m1 = i;
			L1min = L1;
		}
	}

	// there should be two paths. Find the shortest
	double L1 = 0.0;
	for (int i = m0; i != m1;)
	{
		int ip1 = (i + 1) % n;
		double dL = (ch[i] - ch[ip1]).Length();
		L1 += dL;
		++i;
		if (i >= n) i = 0;
	}

	double L2 = 0.0;
	for (int i = m1; i != m0;)
	{
		int ip1 = (i + 1) % n;
		double dL = (ch[i] - ch[ip1]).Length();
		L2 += dL;
		++i;
		if (i >= n) i = 0;
	}

	pt.clear();
	if (L1 < L2)
	{
		for (int i = m0; i != m1;)
		{
			pt.push_back(ch[i]);
			++i;
			if (i >= n) i = 0;
		}
		pt.push_back(ch[m1]);
	}
	else
	{
		for (int i = m1; i != m0;)
		{
			pt.push_back(ch[i]);
			++i;
			if (i >= n) i = 0;
		}
		pt.push_back(ch[m0]);
	}

	// transform back to 3D
	quatd Qi = Q.Inverse();
	for (int i = 0; i < pt.size(); ++i)
	{
		vec3d r = pt[i];
		pt[i] = rc + Qi * r;
	}

	path->m_points = pt;

	return true;
}


bool GLMusclePath::UpdateShortestPath(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	if (m_selNodes.size() < 2)
	{
		path->m_points.push_back(r0);
		path->m_points.push_back(r1);
	}
	else
	{
		// collect the points
		int n = m_selNodes.size();
		vector<vec3d> v; v.reserve(n + 2);
		v.push_back(r0);
		v.push_back(r1);
		for (int i = 0; i < n; ++i)
		{
			int ni = m_selNodes[i];
			vec3d ri = fem.NodePosition(ni, ntime);
			v.push_back(ri);
		}

		// calculate the convex hull
		FEConvexHullMesher mesher;

		path->m_hull = mesher.Create(v);
		if (path->m_hull == nullptr) { return false; }

		// make sure our initial points are still outside
		FEMesh& mesh = *path->m_hull;
		int m0 = -1, m1 = -1;
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			vec3d ri = mesh.Node(i).pos();
			if ((ri - r0).SqrLength() < 1e-12) m0 = i;
			if ((ri - r1).SqrLength() < 1e-12) m1 = i;

			if ((m0 != -1) && ((m1 != -1))) break;
		}
		assert(m0 != -1);
		assert(m1 != -1);

		mesh.TagAllNodes(0);
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& face = mesh.Face(i);
			for (int j = 0; j < face.Nodes(); ++j) mesh.Node(face.n[j]).m_ntag = 1;
		}

		if (mesh.Node(m0).m_ntag != 1) { assert(false); return false; }
		if (mesh.Node(m1).m_ntag != 1) { assert(false); return false; }

		// extract the outide surface
		FEMesh* surf = mesh.ExtractFaces(false);

		m0 = m1 = -1;
		for (int i = 0; i < surf->Nodes(); ++i)
		{
			vec3d ri = surf->Node(i).pos();
			if ((ri - r0).SqrLength() < 1e-12) m0 = i;
			if ((ri - r1).SqrLength() < 1e-12) m1 = i;

			if ((m0 != -1) && ((m1 != -1))) break;
		}
		assert(m0 != -1);
		assert(m1 != -1);

		// calculate the shortest path
		// NOTE: This assumes that the first and last nodes remain at indices 0 and 1 of the surface mesh!
		path->m_points = FindShortestPath(*surf, m0, m1);
	}

	return true;
}

void GLMusclePath::UpdatePathData(int ntime)
{
	if ((ntime < 0) || (ntime >= m_path.size())) return;

	PathData* path = m_path[ntime];
	if (path == nullptr) return;

	// get the path's points
	vector<vec3d>& pt = path->m_points;
	if (pt.empty())
	{
		path->m_data[PathData::PathLength] = 0.0;
		path->m_data[PathData::MomentArm ] = 0.0;
		return;
	}

	// calculate path length
	double L = 0.0;
	for (int i = 0; i < pt.size() - 1; ++i)
	{
		vec3d& r0 = pt[i];
		vec3d& r1 = pt[i + 1];
		L += (r1 - r0).Length();
	}
	path->m_data[PathData::PathLength] = L;

	// calculate moment arm
	if (pt.size() >= 2)
	{
		int n = pt.size();
		vec3d& r0 = pt[n - 2];
		vec3d& r1 = pt[n - 1];

		vec3d c = path->m_ro;

		vec3d e = r1 - r0; e.Normalize();
		vec3d t = r1 - c;
		vec3d m = e ^ t;

		path->m_data[PathData::MomentArm] = m.Length();
	}
	else path->m_data[PathData::MomentArm] = 0.0;
}

bool GLMusclePath::UpdateData(bool bsave)
{
	if (bsave)
	{
		// TODO: This will recalculate everything! 
		Update(GetModel()->CurrentTimeIndex(), 0.f, true);
	}
	return false;
}

double GLMusclePath::DataValue(int field, int step)
{
	// make sure the range is valid
	if ((step < 0) || (step >= m_path.size())) return 0.0;

	// see if we should update the data
	if (m_path[step] == nullptr) UpdatePath(step);

	// get the path
	PathData* path = m_path[step];
	if (path == nullptr) return 0.0;

	// get the data field
	double val = 0.0;
	switch (field)
	{
	case 1: val = path->m_data[PathData::PathLength]; break;
	case 2: val = path->m_data[PathData::MomentArm ]; break;
	}

	// return 
	return val;
}

vec3d GLMusclePath::UpdateOrigin(int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	if (m_closestFace == -1)
	{
		vec3d r0 = GetVecValue(ROTATION_CENTER);
		mesh.TagAllFaces(0);
		int nmin = -1;
		vec3d rmin;
		double L2min;
		// find the closest node on the surface
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& f = mesh.Face(i);
			vec3d ri(0,0,0);
			for (int j = 0; j < f.Nodes(); ++j)
			{
				FENode& nj = mesh.Node(f.n[j]);
				vec3d rj = fem.NodePosition(f.n[j], 0);
				ri += rj;
			}
			ri /= f.Nodes();

			// get the distance
			double L2 = (ri - r0).SqrLength();

			if ((nmin == -1) || (L2 < L2min))
			{
				nmin = i;
				L2min = L2;
				rmin = ri;
			}
		}
		if (nmin == -1) return r0;

		m_closestFace = nmin;

		vec3d dr = r0 - rmin;

		FEFace& f = mesh.Face(nmin);
		vec3d a0 = fem.NodePosition(f.n[0], 0);
		vec3d a1 = fem.NodePosition(f.n[1], 0);
		vec3d a2 = fem.NodePosition(f.n[2], 0);

		vec3d e1 = a1 - a0; e1.Normalize();
		vec3d e2 = a2 - a0; e2.Normalize();
		vec3d e3 = e1 ^ e2; e3.Normalize();
		e2 = e3 ^ e1; e2.Normalize();

		mat3d QT(				\
			e1.x, e1.y, e1.z,	\
			e2.x, e2.y, e2.z,	\
			e3.x, e3.y, e3.z	\
		);

		m_qr = QT * dr;
	}

	// calculate current position of origin
	FEFace& f = mesh.Face(m_closestFace);
	vec3d ri(0, 0, 0);
	for (int j = 0; j < f.Nodes(); ++j)
	{
		FENode& nj = mesh.Node(f.n[j]);
		vec3d rj = fem.NodePosition(f.n[j], ntime);
		ri += rj;
	}
	ri /= f.Nodes();

	vec3d a0 = fem.NodePosition(f.n[0], ntime);
	vec3d a1 = fem.NodePosition(f.n[1], ntime);
	vec3d a2 = fem.NodePosition(f.n[2], ntime);

	vec3d e1 = a1 - a0; e1.Normalize();
	vec3d e2 = a2 - a0; e2.Normalize();
	vec3d e3 = e1 ^ e2; e3.Normalize();
	e2 = e3 ^ e1; e2.Normalize();

	mat3d Q(\
		e1.x, e2.x, e3.x, \
		e1.y, e2.y, e3.y, \
		e1.z, e2.z, e3.z	\
	);

	vec3d dr = Q * m_qr;

	return ri + dr;
}

bool ClosestPointOnRing(CGLModel& glm, int ntime, const vec3d& rc, const vec3d& t, vec3d& q, vec3d& qn)
{
	Post::FEPostModel& fem = *glm.GetFEModel();
	FEPostMesh& mesh = *glm.GetActiveMesh();
	int NF = mesh.Faces();
	int imin = -1;
	double Dmin = 0.0;
	vec3d re[FEFace::MAX_NODES];
	for (int i = 0; i < NF; ++i)
	{
		// get the nodal positions
		FEFace* face = mesh.FacePtr(i);
		int ne = face->Edges();
		for (int j = 0; j < ne; ++j) re[j] = fem.NodePosition(face->n[j], ntime);

		// figure out the case
		double s[FEFace::MAX_NODES] = { 0 };
		int ncase = 0;
		for (int j = 0; j < ne; ++j)
		{
			s[j] = (re[j] - rc) * t;
			if (s[j] > 0) ncase |= (1 << j);
		}

		const int LUT[8][2] = {{-1,-1}, {0,2},{0,1},{1,2},{1,2},{0,1},{0,2},{-1,-1}};

		if ((ncase != 0) && (ncase != 7))
		{
			int e[2] = { LUT[ncase][0], LUT[ncase][1] };

			vec3d er[2];
			for (int j = 0; j < 2; ++j)
			{
				int n = e[j];
				int n0 = n;
				int n1 = (n + 1) % ne;
				vec3d ra = re[n0];
				vec3d rb = re[n1];
				double la = t * (ra - rc);
				double lb = t * (rb - rc);

				assert(la * lb < 0);

				double L = (rb - ra).Length();
				vec3d p = ra + (rb - ra) * (la / L);

				er[j] = p;
			}

			double l = (rc - er[0]) * (er[1] - er[0]) / (er[1] - er[0]).SqrLength();

			vec3d p; 
			if (l <= 0.0) p = er[0];
			else if (l >= 1.0) p = er[1];
			else p = er[0] + (er[1] - er[0]) * l;

			double D2 = (p - rc).SqrLength();
			if ((imin == -1) || (D2 < Dmin))
			{
				imin = i;
				Dmin = D2;
				q = p;

				vec3d fn = face->m_fn;
				qn = fn;
			}
		}
	}

	return (imin != -1);
}

struct RINGPOINT
{
	vec3d	p;	// point on ring
	vec3d	n;	// local surface normal
	vec3d	t;	// normal to ring
};

void ProcessPath(vector<RINGPOINT>& pt)
{
	for (size_t i = 1; i < pt.size() - 1; ++i)
	{
		RINGPOINT& rm = pt[i - 1];
		RINGPOINT& ri = pt[i    ];
		RINGPOINT& rp = pt[i + 1];

		vec3d e1 = ri.p - rm.p;
		vec3d e2 = rp.p - ri.p;

		vec3d t = (e2 - e1); t.Normalize();
		if (t * pt[i].n >= 0.0)
		{
			// remove this point
			pt.erase(pt.begin() + i);
			i--;
		}
	}
}

bool SmoothenPath(Post::CGLModel* glm, int ntime, vector<RINGPOINT>& pt)
{
	// evaluate the initial length
	int NP = pt.size();
	double L0 = 0;
	for (int i = 0; i < NP - 1; ++i)
	{
		RINGPOINT& ri = pt[i];
		RINGPOINT& rp = pt[i + 1];
		L0 += (rp.p - ri.p).Length();
	}

	// see if we can shrink the path
	const int MAX_ITERS = 10;
	int niter = 0;
	bool done = false;
	do
	{
		double DL = 0.0;
		for (int i = 1; i < NP - 1; ++i)
		{
			RINGPOINT& rm = pt[i - 1];
			RINGPOINT& ri = pt[i    ];
			RINGPOINT& rp = pt[i + 1];

			vec3d rc = (rm.p + rp.p) * 0.5;
			vec3d t = (rp.p - rm.p); t.Normalize();

			rc -= t*((rc - ri.p) * t);

			double DL0 = (rp.p - ri.p).Length() + (ri.p - rm.p).Length();

			vec3d q, qn;
			if (ClosestPointOnRing(*glm, ntime, rc, t, q, qn))
			{
				double DL1 = (rp.p - q).Length() + (q - rm.p).Length();
				if (DL1 < DL0)
				{
					ri.p = q;
					ri.n = qn;

					DL += DL0 - DL1;
				}
			}
		}

		done = (DL / L0 < 1e-6);
		niter++;
		if (niter > MAX_ITERS) done = true;
	} 
	while (!done);

	return done;
}

bool GLMusclePath::UpdateSpringPath(PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	vec3d t = r1 - r0; t.Normalize();

	const int STEPS = 50;
	double dr = (r1 - r0).Length() / STEPS;
	vector<RINGPOINT> pt;
	RINGPOINT rp = { r0, vec3d(0,0,0), vec3d(0,0,0) };
	pt.push_back(rp);
	int nsteps = STEPS;
	int n = 1;
	for (int i = 1; i < STEPS; ++i, ++n)
	{
		// get the next point
		vec3d ri = r0 + t * dr;

		// find the closest point on ring
		vec3d q, qn;
		if (ClosestPointOnRing(*glm, ntime, ri, t, q, qn))
		{
			if ((ri - q) * qn < 0.0)
			{
				// if it's inside, project it to the closest point
				RINGPOINT rp = { q, qn, t };
				pt.push_back(rp);
				r0 = q;
				t = r1 - r0; t.Normalize();
				dr = (r1 - r0).Length() / (STEPS - i);

				// process path and remove any points that left contact. 
				ProcessPath(pt);
			}
			else r0 = ri;
		}
		else
		{
			r0 = ri;
		}
	}
	RINGPOINT endPoint = { r1, vec3d(0,0,0) };
	pt.push_back(endPoint);

	// smoothen the path
	SmoothenPath(glm, ntime, pt);

	// copy the points to the PathData
	path->m_points.clear();
	for (size_t i=0; i < pt.size(); ++i) path->m_points.push_back(pt[i].p);

	// all done
	return true;
}
