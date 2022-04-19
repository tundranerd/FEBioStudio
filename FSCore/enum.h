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

#pragma once

// Master Chunk ID
#define CID_MASTER					0x00000000

// Document ID's
#define CID_VERSION					0x00000001

// material library's version
#define CID_MATLIB_VERSION			0x00000001

// Miscellaneous
#define CID_VEC3D					0x00000002

// View Settings
//#define CID_VIEW_SETTINGS			0x00010000
//#define CID_VIEW_CULL				0x00010001
//#define CID_VIEW_CONNECTED			0x00010002
//#define CID_VIEW_PARTBOUNDS			0x00010003
//#define CID_VIEW_AUTOHIDE			0x00010004
//#define CID_VIEW_EXTONLY			0x00010005
//#define CID_VIEW_SOFTSELECT			0x00010006
//#define CID_VIEW_GRID				0x00010007
//#define CID_VIEW_MESH				0x00010008
//#define CID_VIEW_FEATURES			0x00010009
//#define CID_VIEW_NORMALS			0x0001000A
//#define CID_VIEW_FORCES				0x0001000B		// obsolete (1.3?)
//#define CID_VIEW_PRESSURES			0x0001000C		// obsolete (1.3?)
//#define CID_VIEW_JOINTS				0x0001000D
//#define CID_VIEW_FIBERS				0x0001000E
//#define CID_VIEW_LMA				0x0001000F
//#define CID_VIEW_BCCOLOR            0x00010010		// obsolete (1.7)
//#define CID_VIEW_BCTYPE             0x00010011		// obsolete (1.7)
//#define CID_VIEW_CONTOUR			0x00010012
//#define CID_VIEW_BGCOL1				0x00010013
//#define CID_VIEW_BGCOL2				0x00010014
//#define CID_VIEW_FGCOL				0x00010015
//#define CID_VIEW_BGSTYLE			0x00010016
//#define CID_VIEW_NODESIZE			0x00010017
//#define CID_VIEW_RENDER				0x00010018
//#define CID_VIEW_RIGID				0x00010019
//#define CID_VIEW_LINE_SMOOTH		0x0001001A
//#define CID_VIEW_POINT_SMOOTH		0x0001001B
//#define CID_VIEW_CONN_ANGLE			0x0001001C
//#define CID_VIEW_MESH_COLOR			0x0001001D
//#define CID_VIEW_RIGID_WALL			0x0001001E
//#define CID_VIEW_ANGLE_CONSTRAINT	0x0001001F
//#define CID_VIEW_FIBER_SCALE		0x00010020
//#define CID_VIEW_EMULATE_APPLY		0x00010021	// new in 2.0
//#define CID_VIEW_LINEWIDTH			0x00010022	// new in 2.0
//#define CID_VIEW_CLEAR_UNDO			0x00010023	// new in 2.0
//#define CID_VIEW_SHOW_HIDDEN_FIBERS	0x00010024
//#define CID_VIEW_MULTIVIEW_CONV     0x00010025  // new in 2.1

// Project Settings
#define CID_PROJECT					0x00020000
#define CID_PRJ_TITLE				0x00020001
#define CID_PRJ_NTIME				0x00020002
#define CID_PRJ_STEPSIZE			0x00020003
#define CID_PRJ_AUTOTIME			0x00020004
#define CID_PRJ_MXBACK				0x00020005
#define CID_PRJ_ITEOPT				0x00020006
#define CID_PRJ_DTMIN				0x00020007
#define CID_PRJ_DTMAX				0x00020008
#define CID_PRJ_MTHSOL				0x00020009
#define CID_PRJ_MINBANDWIDTH		0x0002000A
#define CID_PRJ_ILIMIT				0x0002000B
#define CID_PRJ_MAXREF				0x0002000C
#define CID_PRJ_DTOL				0x0002000D
#define CID_PRJ_ETOL				0x0002000E
#define CID_PRJ_RTOL				0x0002000F
#define CID_PRJ_TOLLS				0x00020010
#define CID_PRJ_ANALYSIS			0x00020011
#define CID_PRJ_RESTART				0x00020012
#define CID_PRJ_NPLOT				0x00020013
//#define CID_PRJ_PSTIFFNESS			0x00020014	// no longer used
//#define CID_PRJ_SHELLSTRAIN         0x00020015	// no longer used
#define CID_PRJ_MODULES				0x00020016	// obsolete
#define CID_PRJ_MODULE_NAME			0x00020017
#define CID_PRJ_PLOTMAP             0x00020100
#define CID_PRJ_PLOTFIELD1			0x00020101
#define CID_PRJ_PLOTFIELD2			0x00020102
#define CID_PRJ_PLOTFIELD3			0x00020103
#define CID_PRJ_PLOTFIELD4			0x00020104
#define CID_PRJ_OUTGROUPS			0x00020200
#define CID_PRJ_OUTFACES			0x00020201
#define CID_PRJ_MUSTPOINT			0x00020202
#define CID_PRJ_OUTPUT				0x00020300
#define CID_PRJ_OUTPUT_VAR			0x00020310
#define CID_PRJ_OUTPUT_VAR_NAME		0x00020311
#define CID_PRJ_OUTPUT_VAR_ACTIVE	0x00020312
#define CID_PRJ_OUTPUT_VAR_VISIBLE	0x00020313
//#define CID_PRJ_OUTPUT_VAR_DOMAIN	0x00020314	// This was used by the old PreView.
#define CID_PRJ_OUTPUT_VAR_DOMAINID	0x00020315
//#define CID_PRJ_OUTPUT_VAR_MODULE	0x00020316	// obsolete
#define CID_PRJ_LOGDATA				0x00020400
#define CID_PRJ_LOGDATA_ITEM		0x00020401
#define CID_PRJ_LOGDATA_TYPE		0x00020402
#define CID_PRJ_LOGDATA_DATA		0x00020403
#define CID_PRJ_LOGDATA_MID			0x00020404
#define CID_PRJ_LOGDATA_GID			0x00020405
#define CID_PRJ_LOGDATA_CID         0x00020406
#define CID_PRJ_LOGDATA_FILE		0x00020407 // added in FS 1.2

// Model user info
#define CID_MODELINFO				0x00030000
#define CID_MODELINFO_COMMENT		0x00030001
#define CID_MODELINFO_UNITS			0x00030002

// FSModel Data
#define CID_FEM						0x00040000
#define CID_FEM_MUST_POINT			0x00040001		//---> obsolete (1.3)
#define CID_FEM_DATA				0x00041000
#define CID_FEM_CONSTANT			0x00041100
#define CID_FEM_CONST_NAME			0x00041101
#define CID_FEM_CONST_VALUE			0x00041102
#define CID_FEM_SOLUTE_DATA			0x00042000
#define CID_FEM_SOLUTE				0x00042100
#define CID_FEM_SOLUTE_NAME			0x00042101
#define CID_FEM_SOLUTE_CHARGE		0x00042102
#define CID_FEM_SOLUTE_MOLAR_MASS	0x00042103
#define CID_FEM_SOLUTE_DENSITY		0x00042104
#define CID_FEM_SBM_DATA			0x00043000
#define CID_FEM_SBM                 0x00043100
#define CID_FEM_SBM_NAME			0x00043101
#define CID_FEM_SBM_CHARGE          0x00043102
#define CID_FEM_SBM_MOLAR_MASS      0x00043103
#define CID_FEM_SBM_DENSITY         0x00043104

// LoadCurve data
#define CID_LOAD_CURVE				0x00050000
#define CID_LC_ACTIVE				0x00050001		//---> obsolote (1.3)
#define CID_LC_TYPE					0x00050002
#define CID_LC_POINTS				0x00050003
#define CID_LC_POINT				0x00050004
#define CID_LC_TIME					0x00050005
#define CID_LC_LOAD					0x00050006
#define CID_LC_EXTEND				0x00050007
#define CID_LC_FLAGS				0x00050008
#define CID_LC_REF					0x00050009

// Body force data
#define CID_BODY_FORCE				0x00060000	//---> obsolete (1.3)
//#define CID_BODY_FORCE_X			0x00060001
//#define CID_BODY_FORCE_Y			0x00060002
//#define CID_BODY_FORCE_Z			0x00060003

// material data
#define CID_MATERIAL_SECTION		0x00070000
#define CID_MAT_TYPE				0x00070001
#define CID_MAT_ID					0x00070002
#define CID_MAT_NAME				0x00070003
//#define CID_MAT_ELEM				0x00070004	//---> obsolete (1.5)
#define CID_MAT_APPEARANCE			0x00070005
#define CID_MAT_DIFFUSE				0x00070006
#define CID_MAT_AMBIENT				0x00070007
#define CID_MAT_SPECULAR			0x00070008
#define CID_MAT_EMISSION			0x00070009
#define CID_MAT_SHININESS			0x0007000A
#define CID_MAT_PARAMS				0x0007000B
#define CID_MAT_RENDER				0x0007000C
#define CID_MAT_NIKE3D_DATA			0x0007000D
#define CID_MAT_RIGID_PID			0x0007000E
#define CID_MAT_PROPERTY			0x0007000F	
#define CID_MAT_PROPERTY_NAME		0x00070010	// added in 1.13
#define CID_MAT_PROPERTY_MAT		0x00070011	// added in 2.0
#define CID_MAT_AXES				0x00070012
#define CID_MAT_PROPERTY_MATPROP	0x00070013	// added in 2.0

// --- geometry data ---
#define CID_GEOMETRY_SECTION		0x00080000
#define CID_OBJ_ID					0x00080001
#define CID_OBJ_NAME				0x00080002
#define CID_OBJ_HEADER				0x00080003
#define CID_OBJ_POS					0x00080004
#define CID_OBJ_ROT					0x00080005
#define CID_OBJ_PARAMS				0x00080006
#define CID_OBJ_DEFAULT_MESHER		0x00080007	//---> obsolete: FE meshers need to store their types (1.5)
#define CID_OBJ_PARTS				0x00080008
#define CID_OBJ_FACES				0x00080009
#define CID_OBJ_EDGES				0x0008000A
#define CID_OBJ_NODES				0x0008000B
#define CID_OBJ_COLOR				0x0008000C
#define CID_OBJ_SCALE				0x0008000D
#define CID_OBJ_FEMESHER			0x0008000E
#define CID_OBJ_MESH_LAYER			0x0008000F

#define CID_OBJ_PART_LIST			0x00080100
#define CID_OBJ_PART				0x00080101
#define CID_OBJ_PART_ID				0x00080102
#define CID_OBJ_PART_MAT			0x00080103
#define CID_OBJ_PART_NAME			0x00080104	// as of version 1.9
#define CID_OBJ_PART_PARAMS			0x00080105 // as of version 1.4
#define CID_OBJ_PART_SOLIDSECTION	0x00080106
#define CID_OBJ_PART_SHELLSECTION	0x00080107

#define CID_OBJ_SOLID_DOMAIN		0x000801A1
#define CID_OBJ_SHELL_DOMAIN		0x000801A2

#define CID_OBJ_FACE_LIST			0x00080200
#define CID_OBJ_FACE				0x00080201
#define CID_OBJ_FACE_ID				0x00080202
#define CID_OBJ_FACE_PID0			0x00080203
#define CID_OBJ_FACE_PID1			0x00080204
#define CID_OBJ_FACE_NAME			0x00080205	// as of version 1.9
#define CID_OBJ_FACE_TYPE			0x00080206	// as of version 2.0
#define CID_OBJ_FACE_NODELIST		0x00080207	// as of version 2.0
#define CID_OBJ_FACE_EDGELIST		0x00080208	// as of version 2.0

#define CID_OBJ_EDGE_LIST			0x00080300
#define CID_OBJ_EDGE				0x00080301
#define CID_OBJ_EDGE_ID				0x00080302
#define CID_OBJ_EDGE_NAME			0x00080303	// as of version 1.9
#define CID_OBJ_EDGE_TYPE			0x00080304	// as of version 2.0
#define CID_OBJ_EDGE_NODE0			0x00080305	// version 2.0
#define CID_OBJ_EDGE_NODE1			0x00080306	// version 2.0
#define CID_OBJ_EDGE_NODE2			0x00080307	// version 2.0

#define CID_OBJ_NODE_LIST			0x00080400
#define CID_OBJ_NODE				0x00080401
#define CID_OBJ_NODE_ID				0x00080402
#define CID_OBJ_NODE_POS			0x00080403
//#define CID_OBJ_NODE_MESH_NODE	0x00080404	//---> obsolete: GNode no longer stores its corresponding FE node (1.4)
#define CID_OBJ_NODE_NAME			0x00080405	// as of version 1.9

#define CID_OBJ_GOBJECTS			0x00081000
#define CID_OBJ_GPARTGROUP			0x00082000
#define CID_OBJ_GFACEGROUP			0x00083000
#define CID_OBJ_GEDGEGROUP			0x00084000
#define CID_OBJ_GNODEGROUP			0x00085000

#define CID_DISCRETE_OBJECT			0x00086000

#define CID_MESH_LAYERS				0x00087000
#define CID_MESH_LAYER				0x00087100
#define CID_MESH_LAYER_ACTIVE		0x00087101
#define CID_MESH_LAYER_NAME			0x00087102
#define CID_MESH_LAYER_MESH_INDEX	0x00087103

// --- mesh data ---
#define CID_MESH					0x00090000
#define CID_MESH_HEADER				0x00090001
#define CID_MESH_ID					0x00090002
//#define CID_MESH_NAME				0x00090003 //--> obsolete: the name is stored in the object (1.3?)
#define CID_MESH_NODES				0x00090004
#define CID_MESH_ELEMENTS			0x00090005
#define CID_MESH_FACES				0x00090006
#define CID_MESH_EDGES				0x00090007
#define CID_MESH_NODE_SECTION		0x00090008
#define CID_MESH_ELEMENT_SECTION	0x00090009
#define CID_MESH_FACE_SECTION		0x0009000A
#define CID_MESH_EDGE_SECTION		0x0009000B
#define CID_MESH_PART_SECTION		0x0009000C
#define CID_MESH_SURF_SECTION		0x0009000D
#define CID_MESH_NSET_SECTION		0x0009000E
#define CID_MESH_BC_SECTION         0x0009000F
#define CID_MESH_PART				0x00090010
#define CID_MESH_SURFACE			0x00090011
#define CID_MESH_NODESET			0x00090012
#define CID_MESH_PARAMS				0x00090013

#define CID_MESH_NODE				0x00090100
#define CID_MESH_NODE_GID			0x00090101
#define CID_MESH_NODE_POSITION		0x00090102

#define CID_MESH_ELEMENT			0x00090200
#define CID_MESH_ELEMENT_TYPE		0x00090201
#define CID_MESH_ELEMENT_GID		0x00090202
#define CID_MESH_ELEMENT_NODES		0x00090203
#define CID_MESH_ELEMENT_FIBER		0x00090204
#define CID_MESH_SHELL_THICKNESS	0x00090205
//#define CID_MESH_ELEMENT_MATERIAL	0x00090206 // --> obsolete 1.9.1
#define CID_MESH_ELEMENT_Q_ACTIVE	0x00090207
#define CID_MESH_ELEMENT_Q			0x00090208

#define CID_MESH_FACE				0x00090300
#define CID_MESH_FACE_TYPE			0x00090301
#define CID_MESH_FACE_GID			0x00090302
#define CID_MESH_FACE_NODES			0x00090303
#define CID_MESH_FACE_SMOOTHID		0x00090304
#define CID_MESH_FACE_EDGES			0x00090305

#define CID_MESH_EDGE				0x00090400
#define CID_MESH_EDGE_TYPE			0x00090401
#define CID_MESH_EDGE_GID			0x00090402
#define CID_MESH_EDGE_NODES			0x00090403

#define CID_MESH_DATA_SECTION		0x00090500
#define CID_MESH_ELEM_DATA			0x00090501
#define CID_MESH_DATA_NAME			0x00090502
#define CID_MESH_DATA_VALUES		0x00090503
#define CID_MESH_DATA_TYPE			0x00090504
#define CID_MESH_NODE_DATA			0x00090505
#define CID_MESH_SURFACE_DATA		0x00090506
#define CID_MESH_DATA_SURFACE		0x00090507
#define CID_MESH_DATA_PART			0x00090508
#define CID_MESH_DATA_SCALE			0x00090509
#define CID_MESH_PART_DATA			0x0009050A
#define CID_MESH_DATA_NODESET		0x0009050B
#define CID_MESH_DATA_FORMAT		0x0009050C
#define CID_MESH_DATA_DPI			0x0009050D

// --- surface mesh ---
#define CID_SURFACE_MESH			0x00091000

// --- curve mesh ---
#define CID_CURVE_MESH				0x00092000

// --- modifiers ---
#define CID_MODIFIER_STACK			0x000A0000
#define CID_MODIFIER_MESH			0x000A0001
#define CID_MODIFIERS				0x000A0002

// --- FE Objects ---
#define CID_FEOBJ_NAME				0x000B0001	// --> new from version 1.13
#define CID_FEOBJ_PARAMS			0x000B0002	// --> new from version 1.13
#define CID_FEOBJ_INFO				0x000B0003	// --> new in version 3.0

// --- parameters ---
#define CID_PARAM					0x000C0000
#define CID_PARAM_ID				0x000C0001
#define CID_PARAM_TYPE				0x000C0002
#define CID_PARAM_VALUE				0x000C0003
#define CID_PARAM_CHECKED			0x000C0004
#define CID_PARAM_NAME				0x000C0005
#define CID_PARAM_LC				0x000C0006	// new in FBS 2.0

// --- properties ---
// (only used by febio classes)
#define CID_PROPERTY_LIST				0x000C1000
#define CID_PROPERTY					0x000C1001
#define CID_PROPERTY_NAME				0x000C1002
#define CID_PROPERTY_ITEM				0x000C1003

// --- boundary conditions ---
#define CID_BC_SECTION					0x000D0000
#define CID_BC_NAME                     0x000D0001
#define CID_BC_LIST						0x000D0002

// fixed constraint
#define CID_BC_FIXED					0x000D1000
#define CID_BC_FIXED_DATA				0x000D1001

// prescribed constraint
#define CID_BC_PRESCRIBED				0x000D2000
#define CID_BC_PRESCRIBED_DATA			0x000D2001

// prescribed loads
#define CID_BC_NODAL_LOAD				0x000D3000
#define CID_BC_NODAL_LOAD_DATA			0x000D3001

// prescribed pressure
#define CID_BC_SURFACE_LOAD				0x000D4000
#define CID_BC_SURFACE_LOAD_DATA		0x000D4001

// prescribed velocity
#define CID_BC_NODAL_VELOCITIES			0x000D5000
#define CID_BC_NODAL_VELOCITIES_DATA	0x000D5001

// --- contact interfaces ---
#define CID_INTERFACE_SECTION		0x000E0000
#define CID_INTERFACE_NAME			0x000E0001
#define CID_INTERFACE_ACTIVE		0x000E0002
#define CID_INTERFACE_PARAMS		0x000E0003
#define CID_INTERFACE_STEP			0x000E0004
#define CID_INTERFACE_SURFACE2		0x000E0005
#define CID_INTERFACE_SURFACE1		0x000E0006

// rigid interface data
#define CID_RIGID_INTERFACE			0x000E1000
#define CID_RI_LIST					0x000E1001
#define CID_RI_RIGIDBODY			0x000E1002

// sliding interface data
#define CID_SLIDING_INTERFACE		0x000E2000
#define CID_SI_MASTER				0x000E2001
#define CID_SI_SLAVE				0x000E2002

// tied interface data
#define CID_TIED_INTERFACE			0x000E3000
#define CID_TI_MASTER				0x000E3001
#define CID_TI_SLAVE				0x000E3002

// rigid wall data
#define CID_RIGID_WALL				0x000E4000
#define CID_RW_SLAVE				0x000E4001

// rigid joint data
#define CID_RIGID_JOINT				0x000E5000
#define CID_RJ_RIGIDBODY_A			0x000E5001
#define CID_RJ_RIGIDBODY_B			0x000E5002

// biphasic contact
#define CID_BC_INTERFACE			0x000E6000
#define CID_BC_MASTER				0x000E6001
#define CID_BC_SLAVE				0x000E6002

// --- step section ---
#define CID_STEP_SECTION			0x000F0000
#define CID_STEP_NAME				0x000F0001
#define CID_STEP_ID					0x000F0002
#define CID_STEP_PARAMS				0x000F0003
#define CID_STEP_MUST_POINT			0x000F0004
#define CID_STEP_DATA				0x000F0005
#define CID_STEP_PROPERTY			0x000F0006
#define CID_STEP_PROPERTY_NAME		0x000F0007
#define CID_STEP_PROPERTY_TYPESTR	0x000F0008
#define CID_STEP_PROPERTY_DATA		0x000F0009

#define CID_STEP_SETTINGS			0x00100000
#define CID_STEP_NTIME				0x00100001
#define CID_STEP_DT					0x00100002
#define CID_STEP_BAUTO				0x00100003
#define CID_STEP_BMUST				0x00100004
#define CID_STEP_MXBACK				0x00100005
#define CID_STEP_ITEOPT				0x00100006
#define CID_STEP_DTMIN				0x00100007
#define CID_STEP_DTMAX				0x00100008
#define CID_STEP_MTHSOL				0x00100009
#define CID_STEP_BMINBW				0x0010000A
#define CID_STEP_ILIMIT				0x0010000B
#define CID_STEP_MAXREF				0x0010000C
#define CID_STEP_DTOL				0x0010000D
#define CID_STEP_ETOL				0x0010000E
#define CID_STEP_RTOL				0x0010000F
#define CID_STEP_TOLLS				0x00100010
#define CID_STEP_ANALYSIS			0x00100011
#define CID_STEP_PSTIFFNESS			0x00100012	// (obsolete 1.5; changed type of parameter so had to make a new CID entry)
#define CID_STEP_SYMM_PORO			0x00100013	// (obsolete 1.5; replaced by more general matrix storage format
#define CID_STEP_CUTBACK			0x00100014
#define CID_STEP_MINRES				0x00100015
#define CID_STEP_PRES_STIFF			0x00100016	// replaces CID_STEP_PSTIFFNESS
#define CID_STEP_PTOL				0x00100017
#define CID_STEP_CTOL				0x00100018
#define CID_STEP_TITLE				0x00100019
#define CID_STEP_MATFMT				0x0010001A
#define CID_STEP_VTOL				0x0010001B
#define CID_STEP_DIVREF				0x0010001C
#define CID_STEP_REFSTEP			0x0010001D
#define CID_STEP_PLOTSTRIDE			0x0010001E
#define CID_STEP_PLOTLEVEL			0x0010001F

// --- loads ---
#define CID_FC_SECTION				0x00110000

// --- rigid constraints ---
#define CID_RC_SECTION				0x00120000

// --- rigid loads ---- (new in FBS2)
#define CID_RL_SECTION				0x00121000

// --- rigid connectors ---
#define CID_CONNECTOR_SECTION		0x00130000
#define CID_CONNECTOR_NAME			0x00130001
#define CID_CONNECTOR_ACTIVE		0x00130002
#define CID_CONNECTOR_PARAMS		0x00130003
#define CID_CONNECTOR_STEP			0x00130004
#define CID_CONNECTOR_LIST0			0x00130005
#define CID_CONNECTOR_LIST1			0x00130006

// --- initial conditions
#define CID_IC_SECTION				0x00140000

// --- model constraints
#define CID_CONSTRAINT_SECTION		0x00150000

// --- load controllers
#define CID_LOAD_CONTROLLER_LIST	0x00160000

// --- mesh data generators
#define CID_MESHDATA_LIST			0x00170000

// rigid connector joint data
#define CID_RC_SPHERICAL_JOINT		0x000E5000
#define CID_RC_REVOLUTE_JOINT		0x000E5001
#define CID_RC_PRISMATIC_JOINT		0x000E5002
#define CID_RC_CYLINDRICAL_JOINT	0x000E5003
#define CID_RC_PLANAR_JOINT         0x000E5004
#define CID_RC_SPRING               0x000E5005
#define CID_RC_DAMPER               0x000E5006
#define CID_RC_ANGULAR_DAMPER       0x000E5007
#define CID_RC_CONTRACTILE_FORCE    0x000E5008
#define CID_RC_RIGIDBODY_A			0x000E5009
#define CID_RC_RIGIDBODY_B			0x000E5010

// --- FEBio Jobs
#define CID_FEBIOJOB				0x000F0000
#define CID_FEBIOJOB_FILENAME		0x000F0001
#define CID_FEBIOJOB_PLOTFILE		0x000F0002
#define CID_FEBIOJOB_LCONFIG		0x000F0003
#define CID_FEBIOJOB_LOGFILE		0x000F0004

// --- Launch Configs
#define CID_LCONFIG_TYPE			0x000F1000
#define CID_LCONFIG_PATH			0x000F1001
#define CID_LCONFIG_SERVER			0x000F1002
#define CID_LCONFIG_PORT			0x000F1003
#define CID_LCONFIG_USERNAME		0x000F1004
#define CID_LCONFIG_REMOTEDIR		0x000F1005
#define CID_LCONFIG_JOBNAME			0x000F1006
#define CID_LCONFIG_WALLTIME		0x000F1007
#define CID_LCONFIG_PROCNUM			0x000F1008
#define CID_LCONFIG_RAM				0x000F1009
#define CID_LCONFIG_CUSTOMEFILE		0x000F1010
#define CID_LCONFIG_TEXT			0x000F1011

// --- Resources
#define CID_RESOURCE_SECTION		0x00100000
#define CID_RESOURCE_IMAGEMODEL		0x00101000

// --- info for reconstructing FEBio classes
#define CID_FEBIO_META_DATA			0x00200001
#define CID_FEBIO_BASE_DATA			0x00200002
#define CID_FEBIO_TYPE_STRING		0x00200003

//--------------------------------------------------------
// Mesh type
#define FE_MESH					0x0000
#define FE_BOX					0x0001
#define FE_CYLINDER				0x0002
#define FE_SPHERE				0x0003
#define FE_SPRING				0x0004
#define FE_TORUS				0x0005
#define FE_TUBE					0x0006
#define FE_CONE					0x0007
#define FE_HOLLOW_SPHERE		0x0008
#define FE_SHELL_DISC			0x0009
#define FE_SHELL_PATCH			0x000A
#define FE_SHELL_RING			0x000B
#define FE_SHELL_SPHERE			0x000C
#define FE_SHELL_TORUS			0x000D
#define FE_SHELL_TUBE			0x000E
#define FE_MULTI_BLOCK			0x000F
#define FE_CURVE				0x0010
#define FE_CURVE_CIRCLE			0x0011

//--------------------------------------------------------
// Object types
#define GOBJECT				0x0000
#define GBOX				0x0001
#define GCYLINDER			0x0002
#define GSPHERE				0x0003
#define GSPRING				0x0004
#define GTORUS				0x0005
#define GTUBE				0x0006
#define GCONE				0x0007
#define GHOLLOW_SPHERE		0x0008
#define GDISC				0x0009
#define GPATCH				0x000A
#define GRING				0x000B
//#define GSHELL_SPHERE		0x000C	(obsolete 1.5)
//#define GSHELL_TORUS		0x000D	(obsolete 1.5)
#define GSHELL_TUBE			0x000E
#define GMULTI_BLOCK		0x000F
#define GCURVE				0x0010
#define GCURVE_CIRCLE		0x0011
#define GGREGORY_PATCH		0x0012
#define GMESH_OBJECT		0x0013
#define GQUAD_OBJECT		0x0014
#define GPLC_OBJECT			0x0015
#define GMODIFIED_OBJECT	0x0016
#define GSLICE				0x0017	// (from 1.12)
#define GTRUNC_ELLIPSOID	0x0018	// (from 1.13)
#define GQUART_DOG_BONE		0x0019
#define GSOLIDARC			0x001A	// (from 1.14)
#define GCYLINDER_IN_BOX	0x001B	// (from 1.16)
#define GSPHERE_IN_BOX		0x001C	// (from 1.16)
#define GCYLINDER2			0x001D	// (from 1.17)
#define GTUBE2				0x001E	// (from 1.17)
#define GHEXAGON			0x001F	// (from 1.20)
#define GCURVEMESH_OBJECT	0x0020	// from 2.0
#define GSURFACEMESH_OBJECT	0x0021	// from 2.0
#define GOBJECT2D			0x0022	// from 2.0
#define GIMAGEOBJECT		0x0023	// from 2.2
#define GOCCOBJECT			0x0024	// from 2.2
#define GOCC_BOTTLE			0x0025	// from 2.2
#define GOCC_BOX			0x0026	// from 2.2
#define GCYLINDRICAL_PATCH	0x0027	// from FS 1.0
#define GBOX_IN_BOX			0x0028	// from FS 1.0

//--------------------------------------------------------
// Modifier types
#define GMOD_TWIST			0x0001
//#define GMOD_QUAD2TRI		0x0002	(obsolete 1.5.1)
#define GMOD_BEND			0x0003
#define GMOD_SKEW			0x0004
#define GMOD_WRAP			0x0005
#define GMOD_PINCH			0x0006

//--------------------------------------------------------
// Boundary Condition types
#define FE_FIXED_DISPLACEMENT				0x0001
#define FE_PRESCRIBED_DISPLACEMENT			0x0002
#define FE_NODAL_DOF_LOAD					0x0003
#define FE_PRESSURE_LOAD					0x0004
#define FE_INIT_NODAL_VELOCITIES			0x0005
#define FE_SURFACE_TRACTION					0x0006
#define FE_CONST_BODY_FORCE					0x0007
#define FE_FLUID_FLUX						0x0008
#define FE_FIXED_FLUID_PRESSURE				0x0009
#define FE_PRESCRIBED_FLUID_PRESSURE		0x000A
#define FE_FIXED_TEMPERATURE				0x000B
#define FE_PRESCRIBED_TEMPERATURE			0x000C
#define FE_FIXED_CONCENTRATION				0x000D
#define FE_PRESCRIBED_CONCENTRATION			0x000E
#define FE_HEAT_FLUX						0x000F
#define FE_SOLUTE_FLUX						0x0010
#define FE_INIT_CONCENTRATION				0x0011
#define FE_INIT_FLUID_PRESSURE				0x0012
#define FE_BP_NORMAL_TRACTION				0x0013
#define FE_CONV_HEAT_FLUX					0x0014
#define FE_HEAT_SOURCE						0x0015
#define FE_INIT_TEMPERATURE					0x0016
#define FE_FIXED_SOLID_VELOCITY				0x0017
#define FE_FIXED_FLUID_DILATATION			0x0018
#define FE_PRESCRIBED_SOLID_VELOCITY		0x0019
#define FE_PRESCRIBED_FLUID_DILATATION		0x001A
#define FE_FLUID_TRACTION					0x001B
#define FE_FIXED_ROTATION					0x001C	// added in 1.19
#define FE_PRESCRIBED_ROTATION				0x001D	// added in 1.19
#define FE_INIT_SHELL_FLUID_PRESSURE		0x001E
#define FE_INIT_SHELL_CONCENTRATION			0x001F
#define FE_FIXED_FLUID_VELOCITY				0x0020
#define FE_PRESCRIBED_FLUID_VELOCITY		0x0021
#define FE_FLUID_NORMAL_VELOCITY			0x0022
#define FE_FLUID_FLOW_RESISTANCE			0x0023
#define FE_FLUID_BACKFLOW_STABIL			0x0024
#define FE_FLUID_TANGENTIAL_STABIL			0x0025
#define FE_FLUID_VELOCITY					0x0026
#define FE_FLUID_ROTATIONAL_VELOCITY		0x0027
#define FE_FSI_TRACTION						0x0028
#define FE_FIXED_SHELL_DISPLACEMENT			0x0029	// added in 2.0
#define FE_PRESCRIBED_SHELL_DISPLACEMENT	0x002A	// added in 2.0
#define FE_INIT_NODAL_SHELL_VELOCITIES		0x002B	// added in 2.0
#define FE_CONCENTRATION_FLUX				0x002C	// added in 2.0
#define FE_SBM_POINT_SOURCE					0x002D
#define FE_INIT_FLUID_DILATATION            0x002E
#define FE_FLUID_FLOW_RCR                   0x002F
#define FE_MATCHING_OSM_COEF                0x0030
#define FE_NON_CONST_BODY_FORCE				0x0031
#define FE_INIT_PRESTRAIN					0x0032
#define FE_CENTRIFUGAL_BODY_FORCE           0x0033
#define FE_BFSI_TRACTION                    0x0034
#define FE_FLUID_PRESSURE_LOAD              0x0035
#define FE_MASSDAMPING_LOAD					0x0036

#define FE_FEBIO_BC							0x0036
#define FE_FEBIO_NODAL_LOAD					0x0037
#define FE_FEBIO_SURFACE_LOAD				0x0038
#define FE_FEBIO_BODY_LOAD					0x0039
#define FE_FEBIO_INITIAL_CONDITION			0x003A
#define FE_FEBIO_RIGID_LOAD					0x003B

//--------------------------------------------------------
// Interface types
#define FE_RIGID_INTERFACE				0x0001
#define FE_SLIDING_INTERFACE			0x0002
#define FE_RIGID_JOINT					0x0003
#define FE_RIGID_WALL					0x0004
#define FE_TIED_INTERFACE				0x0005
#define FE_PORO_INTERFACE				0x0006
#define FE_PORO_SOLUTE_INTERFACE		0x0007
#define FE_TENSCOMP_INTERFACE			0x0008
#define FE_TIEDBIPHASIC_INTERFACE		0x0009
#define FE_SPRINGTIED_INTERFACE			0x000A
#define FE_MULTIPHASIC_INTERFACE		0x000B
#define FE_STICKY_INTERFACE				0x000C
#define FE_PERIODIC_BOUNDARY			0x000D
#define FE_RIGID_SPHERE_CONTACT			0x000F	// added in 1.20
#define FE_SLIDING_WITH_GAPS			0x0011	// added in 2.0
#define FE_FACET_ON_FACET_SLIDING		0x0012	// added in 2.0
#define FE_FACET_ON_FACET_TIED			0x0013	// added in 2.0
#define FE_TIEDMULTIPHASIC_INTERFACE	0x0014	// added in 2.0
#define FE_TIED_ELASTIC_INTERFACE       0x0016	// added in 2.0
#define FE_GAPHEATFLUX_INTERFACE		0x0017	// added in 2.1
#define FE_CONTACTPOTENTIAL_CONTACT		0x0018

#define FE_FEBIO_INTERFACE				0x0100

//--------------------------------------------------------
// Connector types
#define FE_RC_SPHERICAL_JOINT		0x0001
#define FE_RC_REVOLUTE_JOINT		0x0002
#define FE_RC_PRISMATIC_JOINT		0x0003
#define FE_RC_CYLINDRICAL_JOINT		0x0004
#define FE_RC_PLANAR_JOINT          0x0005
#define FE_RC_SPRING                0x0006
#define FE_RC_DAMPER                0x0007
#define FE_RC_ANGULAR_DAMPER		0x0008
#define FE_RC_CONTRACTILE_FORCE		0x0009
#define FE_RC_RIGID_LOCK            0x000A
#define FE_RC_GENERIC_JOINT			0x000B
#define FE_FEBIO_RIGID_CONNECTOR	0x000C

//--------------------------------------------------------
// Constraint types
#define FE_SYMMETRY_PLANE				0x0010	// added in 2.0
#define FE_NORMAL_FLUID_FLOW            0x0015	// added in 2.0
#define FE_VOLUME_CONSTRAINT			0x000E	// added in 1.17
#define FE_WARP_CONSTRAINT				0x002E
#define FE_FRICTIONLESS_FLUID_WALL      0x002F  // added in 2.0
#define FE_INSITUSTRETCH_CONSTRAINT		0x0030	// added in FS 1.0
#define FE_PRESTRAIN_CONSTRAINT		    0x0031	// added in FS 1.0
#define FE_FEBIO_NLCONSTRAINT			0x0032	// added in FS 2.0
#define FE_FEBIO_SURFACECONSTRAINT		0x0033	// added in FS 2.0
#define FE_FEBIO_BODYCONSTRAINT			0x0034	// added in FS 2.0
