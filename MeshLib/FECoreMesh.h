#pragma once
#include "FENode.h"
#include "FEElement.h"
#include "FEMeshBase.h"
#include <vector>

//-----------------------------------------------------------------------------
//! This class defines a simple mesh structure that provides basic container
//! services for storing mesh data. It only stores nodes, edges, faces. It implements 
//! an interface for accessing element data, but derived classes need to implement this. 
class FECoreMesh : public ::FEMeshBase
{
public:
	//! constructor
	FECoreMesh();

	//! destructor
	virtual ~FECoreMesh();

	//! allocate space for mesh
	virtual void Create(int nodes, int elems, int faces = 0, int edges = 0) = 0;

	//! check the type of the mesh
	bool IsType(int ntype) const;

public: // interface for accessing elements

	//! total number of elements
	virtual int Elements() const = 0;

	//! return reference to element
	virtual FEElement_& ElementRef(int n) = 0;
	virtual const FEElement_& ElementRef(int n) const = 0;

	//! return pointer to element
	FEElement_* ElementPtr(int n = 0);
	const FEElement_* ElementPtr(int n = 0) const;

	// tag all elements
	void TagAllElements(int ntag);

	// select a list of elements
	void SelectElements(const vector<int>& elem);

public:
	void ShowElements(vector<int>& elem, bool show = true);
	void UpdateItemVisibility();
	void ShowAllElements();

public:
	vec3d ElementCenter(FEElement_& el) const;

	// get the local positions of an element
	void ElementNodeLocalPositions(const FEElement_& e, vec3d* r) const;

	// element volume
	double ElementVolume(int iel);
	double HexVolume(const FEElement_& el);
	double PentaVolume(const FEElement_& el);
	double TetVolume(const FEElement_& el);
	double PyramidVolume(const FEElement_& el);

	//! see if this is a shell mesh
	bool IsShell() const;

	//! see if this is a solid mesh
	bool IsSolid() const;

	//! return nr of shell elements
	int ShellElements();

	//! return nr of solid elements
	int SolidElements();

	//! return nr of beam elements
	int BeamElements();

	//! Is an element exterior or not
	bool IsExterior(FEElement_* pe) const;

	// find a face of an element
	int FindFace(FEElement_* pe, FEFace& f, FEFace& fe);

	// Find and label all exterior nodes
	void MarkExteriorNodes();

	// returns a list of node indices that belong to a part with part ID gid
	void FindNodesFromPart(int gid, vector<int>& node);

	int CountNodePartitions() const;
	int CountEdgePartitions() const;
	int CountFacePartitions() const;
	int CountElementPartitions() const;
	int CountSmoothingGroups() const;
};

inline FEElement_* FECoreMesh::ElementPtr(int n) { return ((n >= 0) && (n<Elements()) ? &ElementRef(n) : 0); }
inline const FEElement_* FECoreMesh::ElementPtr(int n) const { return ((n >= 0) && (n<Elements()) ? &ElementRef(n) : 0); }

// --- I N T E G R A T E ---
double IntegrateQuad(vec3d* r, float* v);
float IntegrateQuad(vec3f* r, float* v);
float IntegrateHex(vec3f* r, float* v);
