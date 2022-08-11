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
#include "FEProject.h"
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FELoadController.h>
#include <FEMLib/FEMeshDataGenerator.h>
#include <FEMLib/FEElementFormulation.h>
#include "GGroup.h"
#include "GModel.h"
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GObject.h>
#include <sstream>
//using namespace std;

//=================================================================================================
CLogDataSettings::CLogDataSettings()
{
}

void CLogDataSettings::RemoveLogData(int item)
{
	m_log.erase(m_log.begin() + item);
}

void CLogDataSettings::Save(OArchive& ar)
{
	const int N = (int) m_log.size();
	for (int i = 0; i<N; ++i)
	{
		FELogData& v = m_log[i];
		ar.BeginChunk(CID_PRJ_LOGDATA_ITEM);
		{
			ar.WriteChunk(CID_PRJ_LOGDATA_TYPE, v.type);
			ar.WriteChunk(CID_PRJ_LOGDATA_DATA, v.sdata);
			ar.WriteChunk(CID_PRJ_LOGDATA_MID , v.matID);
			ar.WriteChunk(CID_PRJ_LOGDATA_GID , v.groupID);
            ar.WriteChunk(CID_PRJ_LOGDATA_CID , v.rcID);
			ar.WriteChunk(CID_PRJ_LOGDATA_FILE, v.fileName);
		}
		ar.EndChunk();
	}
}

void CLogDataSettings::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		if (ar.GetChunkID() == CID_PRJ_LOGDATA_ITEM)
		{
			string data, file;
			int ntype;
			int mid = -1, gid = -1, cid = -1;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PRJ_LOGDATA_TYPE: ar.read(ntype); break;
				case CID_PRJ_LOGDATA_DATA: ar.read(data); break;
				case CID_PRJ_LOGDATA_MID : ar.read(mid); break;
				case CID_PRJ_LOGDATA_GID : ar.read(gid); break;
                case CID_PRJ_LOGDATA_CID : ar.read(cid); break;
				case CID_PRJ_LOGDATA_FILE: ar.read(file); break;
				}
				ar.CloseChunk();
			}

			FELogData d;
			d.type = ntype;
			d.sdata = data;
			d.matID = mid;
			d.groupID = gid;
            d.rcID = cid;
			d.fileName = file;
			AddLogData(d);
		}
		ar.CloseChunk();
	}
}

//=================================================================================================
//-----------------------------------------------------------------------------
// constructor
FSProject::FSProject(void) : m_plt(*this)
{
	// set the title
	m_title = "untitled";

	m_module = -1;

	m_units = 0; // 0 = no unit system

	static bool init = false;
	if (init == false)
	{
		InitModules();
		init = true;
	}
}

//-----------------------------------------------------------------------------
// destructor
FSProject::~FSProject(void)
{
	
}

//-----------------------------------------------------------------------------
void FSProject::Reset()
{
	m_fem.New();
	m_plt.Init();
}

//-----------------------------------------------------------------------------
int FSProject::GetModule() const
{
	return m_module;// FEBio::GetActiveModule();
}

//-----------------------------------------------------------------------------
std::string FSProject::GetModuleName() const
{
	int mod = FEBio::GetActiveModule();
	assert(mod == m_module);
	return FEBio::GetModuleName(mod);
}

//-----------------------------------------------------------------------------
void FSProject::SetModule(int mod, bool setDefaultPlotVariables)
{
	m_module = mod;
	FEBio::SetActiveModule(mod);

	// get the list of variables
	if (mod != -1)
	{
		FSModel& fem = GetFSModel();
		FEBio::InitFSModel(fem);

		if (setDefaultPlotVariables && (m_plt.PlotVariables() == 0))
		{
			// add some default variables
			// TODO: Maybe I can pull this info from FEBio somehow
			SetDefaultPlotVariables();
		}
	}
}

//-----------------------------------------------------------------------------
void FSProject::SetTitle(const std::string& title)
{ 
	m_title = title; 
}

//-----------------------------------------------------------------------------
// save project data to archive
void FSProject::Save(OArchive& ar)
{
	// save the title
	ar.WriteChunk(CID_PRJ_TITLE   , m_title);

	// save the modules flag
	int module = FEBio::GetActiveModule();
	string modName(FEBio::GetModuleName(module));
	ar.WriteChunk(CID_PRJ_MODULE_NAME, modName);

	// save the model data
	ar.BeginChunk(CID_FEM);
	{
		m_fem.Save(ar);
	}
	ar.EndChunk();

	// save the output data
	int N = m_plt.PlotVariables();
	if (N > 0)
	{
		ar.BeginChunk(CID_PRJ_OUTPUT);
		{
			m_plt.Save(ar);
		}
		ar.EndChunk();
	}

	// save the logfile data
	N = m_log.LogDataSize();
	if (N > 0)
	{
		ar.BeginChunk(CID_PRJ_LOGDATA);
		{
			m_log.Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
int MapOldToNewModules(int oldId)
{
	if (oldId & MODULE_FLUID_FSI  ) return FEBio::GetModuleId("fluid-FSI");
	if (oldId & MODULE_FLUID      ) return FEBio::GetModuleId("fluid");
	if (oldId & MODULE_MULTIPHASIC) return FEBio::GetModuleId("multiphasic");
	if (oldId & MODULE_SOLUTES    ) return FEBio::GetModuleId("solute");
	if (oldId & MODULE_BIPHASIC   ) return FEBio::GetModuleId("biphasic");
	if (oldId & MODULE_HEAT       ) return FEBio::GetModuleId("heat");
	if (oldId & MODULE_MECH       ) return FEBio::GetModuleId("solid");
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
// load project data from archive
void FSProject::Load(IArchive &ar)
{
	TRACE("FSProject::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_PRJ_TITLE  : ar.read(m_title); break;
		case CID_PRJ_MODULES: { 
			int oldModuleId = 0;  
			ar.read(oldModuleId); 
			int moduleId = MapOldToNewModules(oldModuleId);
			assert(moduleId != -1);
			SetModule(moduleId);
		} 
		break;
		case CID_PRJ_MODULE_NAME:
		{
			string modName;
			ar.read(modName);
			int moduleId = FEBio::GetModuleId(modName); assert(moduleId > 0);
			SetModule(moduleId);
		}
		break;
		case CID_FEM        : m_fem.Load(ar); break;
		case CID_PRJ_OUTPUT : m_plt.Load(ar); break;
		case CID_PRJ_LOGDATA: m_log.Load(ar); break;
		}
		ar.CloseChunk();
	}

	if (ar.Version() < 0x00040000)
	{
		ar.log("Converting FE model:");
		ar.log("===================");
		std::ostringstream log;
		ConvertToNewFormat(log);
		string s = log.str();
		if (s.empty() == false) ar.log(s.c_str());
		else ar.log("No issues found!");
	}
}

//-----------------------------------------------------------------------------
// Validate the project, that is try to catch problems that FEBio won't like
// returns 0 on success, non-zero on failure
int FSProject::Validate(std::string &szerr)
{
	szerr = "ERRORS:\n";
	GModel& mdl = m_fem.GetModel();

	// --- collect metrics ---
	int nerrs = 0;

	// number of objects
	int nobj = mdl.Objects();
	if (nobj   ==  0) { szerr += "-No geometry is defined for this model.\n"; nerrs++; }

	// parts that don't have a material assigned
	int npnm = 0; 
	for (int i=0; i<mdl.Parts(); ++i)
	{
		GPart* pg = mdl.Part(i);
		if (pg->GetMaterialID() == -1) npnm++;
	}
	if (npnm   >   0) { szerr += "-Not all parts or objects have a material assigned.\n"; nerrs++; }

	// numer of analysis steps
	int nsteps = m_fem.Steps() - 1;
	if (nsteps <=  0) { szerr += "-At least one analysis step is required to run the model.\n"; nerrs++; }

	// number of objects that have no mesh
	int nnomesh = 0;
	for (int i=0; i<mdl.Objects(); ++i)
	{
		FSMesh* pm = mdl.Object(i)->GetFEMesh();
		if (pm == 0) nnomesh++;
	}
	if (nnomesh > 0) { szerr += "-Not all objects have a mesh.\n"; nerrs++; }

	return nerrs;
}

void FSProject::InitModules()
{
	// register material categories
	FEMaterialFactory::AddCategory("elastic"             , MODULE_MECH               , FE_MAT_ELASTIC);
	FEMaterialFactory::AddCategory("uncoupled elastic"   , MODULE_MECH               , FE_MAT_ELASTIC_UNCOUPLED);
	FEMaterialFactory::AddCategory("bi-/tri-/multiphasic", MODULE_BIPHASIC           , FE_MAT_MULTIPHASIC);
	FEMaterialFactory::AddCategory("heat transfer"       , MODULE_HEAT               , FE_MAT_HEAT_TRANSFER);
	FEMaterialFactory::AddCategory("fluid"               , MODULE_FLUID              , FE_MAT_FLUID);
    FEMaterialFactory::AddCategory("fluid-FSI"           , MODULE_FLUID_FSI          , FE_MAT_FLUID_FSI);
	FEMaterialFactory::AddCategory("reaction-diffusion"  , MODULE_REACTION_DIFFUSION , FE_MAT_REACTION_DIFFUSION);
	FEMaterialFactory::AddCategory("other"               , MODULE_MECH				 , FE_MAT_RIGID);

	// --- MECH MODULE ---
	REGISTER_FE_CLASS(FSNonLinearMechanics         , MODULE_MECH, FEANALYSIS_ID         , FE_STEP_MECHANICS               , "Structural Mechanics");
	REGISTER_FE_CLASS(FSFixedDisplacement          , MODULE_MECH, FEBC_ID			    , FE_FIXED_DISPLACEMENT           , "Fixed displacement");
	REGISTER_FE_CLASS(FSFixedShellDisplacement     , MODULE_MECH, FEBC_ID			    , FE_FIXED_SHELL_DISPLACEMENT     , "Fixed shell displacement");
	REGISTER_FE_CLASS(FSFixedRotation              , MODULE_MECH, FEBC_ID			    , FE_FIXED_ROTATION               , "Fixed shell rotation");
	REGISTER_FE_CLASS(FSPrescribedDisplacement     , MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_DISPLACEMENT      , "Prescribed displacement");
	REGISTER_FE_CLASS(FSPrescribedShellDisplacement, MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_SHELL_DISPLACEMENT, "Prescribed shell displacement");
	REGISTER_FE_CLASS(FSPrescribedRotation         , MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_ROTATION          , "Prescribed shell rotation");
	REGISTER_FE_CLASS(FSPressureLoad               , MODULE_MECH, FELOAD_ID             , FE_PRESSURE_LOAD                , "Pressure");
	REGISTER_FE_CLASS(FSSurfaceTraction            , MODULE_MECH, FELOAD_ID             , FE_SURFACE_TRACTION             , "Surface traction");
	REGISTER_FE_CLASS(FSSurfaceForceUniform        , MODULE_MECH, FELOAD_ID             , FE_SURFACE_FORCE                , "Surface force");
	REGISTER_FE_CLASS(FSBearingLoad                , MODULE_MECH, FELOAD_ID             , FE_BEARING_LOAD                 , "Bearing load");
	REGISTER_FE_CLASS(FSNodalVelocities            , MODULE_MECH, FEIC_ID               , FE_INIT_NODAL_VELOCITIES        , "Velocity");
	REGISTER_FE_CLASS(FSNodalShellVelocities       , MODULE_MECH, FEIC_ID               , FE_INIT_NODAL_SHELL_VELOCITIES  , "Shell velocity");
	REGISTER_FE_CLASS(FSInitPrestrain              , MODULE_MECH, FEIC_ID               , FE_INIT_PRESTRAIN               , "Initialize Prestrain");
	REGISTER_FE_CLASS(FSRigidInterface             , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_INTERFACE              , "Rigid");
	REGISTER_FE_CLASS(FSSlidingWithGapsInterface   , MODULE_MECH, FESURFACEINTERFACE_ID , FE_SLIDING_WITH_GAPS            , "Sliding node-on-facet");
	REGISTER_FE_CLASS(FSFacetOnFacetInterface      , MODULE_MECH, FESURFACEINTERFACE_ID , FE_FACET_ON_FACET_SLIDING       , "Sliding facet-on-facet");
    REGISTER_FE_CLASS(FSTensionCompressionInterface, MODULE_MECH, FESURFACEINTERFACE_ID , FE_TENSCOMP_INTERFACE           , "Sliding elastic");
	REGISTER_FE_CLASS(FSTiedInterface              , MODULE_MECH, FESURFACEINTERFACE_ID , FE_TIED_INTERFACE               , "Tied node-on-facet");
	REGISTER_FE_CLASS(FSF2FTiedInterface           , MODULE_MECH, FESURFACEINTERFACE_ID , FE_FACET_ON_FACET_TIED          , "Tied facet-on-facet");
    REGISTER_FE_CLASS(FSTiedElasticInterface       , MODULE_MECH, FESURFACEINTERFACE_ID , FE_TIED_ELASTIC_INTERFACE       , "Tied elastic");
    REGISTER_FE_CLASS(FSContactPotentialInterface  , MODULE_MECH, FESURFACEINTERFACE_ID , FE_CONTACTPOTENTIAL_CONTACT     , "contact potential");
	REGISTER_FE_CLASS(FSStickyInterface            , MODULE_MECH, FESURFACEINTERFACE_ID , FE_STICKY_INTERFACE             , "Sticky");
	REGISTER_FE_CLASS(FSPeriodicBoundary           , MODULE_MECH, FESURFACEINTERFACE_ID , FE_PERIODIC_BOUNDARY            , "Periodic boundary");
	REGISTER_FE_CLASS(FSRigidWallInterface         , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_WALL                   , "Rigid wall");
	REGISTER_FE_CLASS(FSRigidSphereInterface       , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_SPHERE_CONTACT         , "Rigid sphere");
	REGISTER_FE_CLASS(FSRigidJoint                 , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_JOINT                  , "Rigid joint");
	REGISTER_FE_CLASS(FSConstBodyForce             , MODULE_MECH, FELOAD_ID        , FE_CONST_BODY_FORCE             , "Const body force");
	REGISTER_FE_CLASS(FSNonConstBodyForce          , MODULE_MECH, FELOAD_ID        , FE_NON_CONST_BODY_FORCE         , "Non-const body force");
    REGISTER_FE_CLASS(FSCentrifugalBodyForce       , MODULE_MECH, FELOAD_ID        , FE_CENTRIFUGAL_BODY_FORCE       , "Centrifugal body force");
    REGISTER_FE_CLASS(FSMassDamping                , MODULE_MECH, FELOAD_ID        , FE_MASSDAMPING_LOAD             , "Mass damping");

	REGISTER_FE_CLASS(FSRigidFixed			, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_FIXED				, "Fixed rigid displacement/rotation");
	REGISTER_FE_CLASS(FSRigidDisplacement	, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_DISPLACEMENT		, "Prescribed rigid displacement/rotation");
	REGISTER_FE_CLASS(FSRigidForce			, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_FORCE				, "Prescribed rigid force");
	REGISTER_FE_CLASS(FSRigidVelocity		, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_INIT_VELOCITY		, "Initial rigid velocity");
	REGISTER_FE_CLASS(FSRigidAngularVelocity, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_INIT_ANG_VELOCITY	, "Initial rigid angular velocity");

	REGISTER_FE_CLASS(FSRigidSphericalJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_SPHERICAL_JOINT	, "Spherical joint");
	REGISTER_FE_CLASS(FSRigidRevoluteJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_REVOLUTE_JOINT		, "Revolute joint");
	REGISTER_FE_CLASS(FSRigidPrismaticJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_PRISMATIC_JOINT	, "Prismatic joint");
	REGISTER_FE_CLASS(FSRigidCylindricalJoint	, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_CYLINDRICAL_JOINT	, "Cylindrical joint");
	REGISTER_FE_CLASS(FSRigidPlanarJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_PLANAR_JOINT		, "Planar joint");
    REGISTER_FE_CLASS(FSRigidLock               , MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_RIGID_LOCK         , "Rigid lock");
	REGISTER_FE_CLASS(FSRigidSpring				, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_SPRING				, "Spring");
	REGISTER_FE_CLASS(FSRigidDamper				, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_DAMPER				, "Damper");
	REGISTER_FE_CLASS(FSRigidAngularDamper		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_ANGULAR_DAMPER		, "Angular damper");
	REGISTER_FE_CLASS(FSRigidContractileForce	, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_CONTRACTILE_FORCE	, "Contractile force");
	REGISTER_FE_CLASS(FSGenericRigidJoint       , MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_GENERIC_JOINT      , "Generic joint");

	REGISTER_FE_CLASS(FSSymmetryPlane      , MODULE_MECH, FENLCONSTRAINT_ID, FE_SYMMETRY_PLANE      , "symmetry plane");
	REGISTER_FE_CLASS(FSVolumeConstraint   , MODULE_MECH, FENLCONSTRAINT_ID, FE_VOLUME_CONSTRAINT   , "volume constraint");
	REGISTER_FE_CLASS(FSWarpingConstraint  , MODULE_MECH, FENLCONSTRAINT_ID, FE_WARP_CONSTRAINT     , "warp-image");
	REGISTER_FE_CLASS(FSPrestrainConstraint, MODULE_MECH, FENLCONSTRAINT_ID, FE_PRESTRAIN_CONSTRAINT, "prestrain");
	REGISTER_FE_CLASS(FSInSituStretchConstraint, MODULE_MECH, FENLCONSTRAINT_ID, FE_INSITUSTRETCH_CONSTRAINT, "in-situ stretch");
	REGISTER_FE_CLASS(FSFixedNormalDisplacement, MODULE_MECH, FENLCONSTRAINT_ID, FE_FIXED_NORMAL_DISPLACEMENT, "fixed normal displacement");

	// --- HEAT MODULE ---
	REGISTER_FE_CLASS(FSHeatTransfer         , MODULE_HEAT, FEANALYSIS_ID        , FE_STEP_HEAT_TRANSFER    , "Heat Transfer");
	REGISTER_FE_CLASS(FSFixedTemperature     , MODULE_HEAT, FEBC_ID              , FE_FIXED_TEMPERATURE     , "Zero temperature");
	REGISTER_FE_CLASS(FSPrescribedTemperature, MODULE_HEAT, FEBC_ID              , FE_PRESCRIBED_TEMPERATURE, "Prescribed temperature");
	REGISTER_FE_CLASS(FSHeatFlux             , MODULE_HEAT, FELOAD_ID       , FE_HEAT_FLUX             , "Heat flux");
	REGISTER_FE_CLASS(FSConvectiveHeatFlux   , MODULE_HEAT, FELOAD_ID       , FE_CONV_HEAT_FLUX        , "Convective heat flux");
	REGISTER_FE_CLASS(FSInitTemperature      , MODULE_HEAT, FEIC_ID              , FE_INIT_TEMPERATURE      , "Temperature");
	REGISTER_FE_CLASS(FSHeatSource           , MODULE_HEAT, FELOAD_ID       , FE_HEAT_SOURCE           , "Heat source");
	REGISTER_FE_CLASS(FSGapHeatFluxInterface , MODULE_HEAT, FESURFACEINTERFACE_ID, FE_GAPHEATFLUX_INTERFACE , "Gap heat flux");

	// --- BIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSNonLinearBiphasic      , MODULE_BIPHASIC, FEANALYSIS_ID        , FE_STEP_BIPHASIC            , "Biphasic");
	REGISTER_FE_CLASS(FSFixedFluidPressure     , MODULE_BIPHASIC, FEBC_ID              , FE_FIXED_FLUID_PRESSURE     , "Zero fluid pressure");
	REGISTER_FE_CLASS(FSPrescribedFluidPressure, MODULE_BIPHASIC, FEBC_ID              , FE_PRESCRIBED_FLUID_PRESSURE, "Prescribed fluid pressure");
	REGISTER_FE_CLASS(FSFluidFlux              , MODULE_BIPHASIC, FELOAD_ID       , FE_FLUID_FLUX               , "Fluid flux");
	REGISTER_FE_CLASS(FSBPNormalTraction       , MODULE_BIPHASIC, FELOAD_ID       , FE_BP_NORMAL_TRACTION       , "Mixture normal traction");
	REGISTER_FE_CLASS(FSInitFluidPressure      , MODULE_BIPHASIC, FEIC_ID              , FE_INIT_FLUID_PRESSURE      , "Fluid pressure");
    REGISTER_FE_CLASS(FSInitShellFluidPressure , MODULE_BIPHASIC, FEIC_ID              , FE_INIT_SHELL_FLUID_PRESSURE, "Shell fluid pressure");
    REGISTER_FE_CLASS(FSPoroContact            , MODULE_BIPHASIC, FESURFACEINTERFACE_ID, FE_PORO_INTERFACE           , "Biphasic contact");
    REGISTER_FE_CLASS(FSTiedBiphasicInterface  , MODULE_BIPHASIC, FESURFACEINTERFACE_ID, FE_TIEDBIPHASIC_INTERFACE   , "Tied biphasic contact");

	// --- SOLUTES MODULE ---
	REGISTER_FE_CLASS(FSFixedConcentration      , MODULE_SOLUTES, FEBC_ID, FE_FIXED_CONCENTRATION      , "Zero concentration");
	REGISTER_FE_CLASS(FSPrescribedConcentration , MODULE_SOLUTES, FEBC_ID, FE_PRESCRIBED_CONCENTRATION , "Prescribed concentration");
	REGISTER_FE_CLASS(FSInitConcentration       , MODULE_SOLUTES, FEIC_ID, FE_INIT_CONCENTRATION       , "Concentration");
    REGISTER_FE_CLASS(FSInitShellConcentration  , MODULE_SOLUTES, FEIC_ID, FE_INIT_SHELL_CONCENTRATION , "Shell concentration");

	// --- MULTIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSBiphasicSolutes           , MODULE_MULTIPHASIC, FEANALYSIS_ID        , FE_STEP_BIPHASIC_SOLUTE     , "Biphasic-solute");
	REGISTER_FE_CLASS(FSMultiphasicAnalysis       , MODULE_MULTIPHASIC, FEANALYSIS_ID        , FE_STEP_MULTIPHASIC         , "Multiphasic");
	REGISTER_FE_CLASS(FSPoroSoluteContact         , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_PORO_SOLUTE_INTERFACE    , "Biphasic-solute contact");
	REGISTER_FE_CLASS(FSMultiphasicContact        , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_MULTIPHASIC_INTERFACE    , "Multiphasic contact");
	REGISTER_FE_CLASS(FSTiedMultiphasicInterface  , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_TIEDMULTIPHASIC_INTERFACE, "Tied multiphasic contact");
	REGISTER_FE_CLASS(FSSoluteFlux                , MODULE_MULTIPHASIC, FELOAD_ID            , FE_SOLUTE_FLUX              , "Solute flux");
	REGISTER_FE_CLASS(FSSoluteNaturalFlux         , MODULE_MULTIPHASIC, FELOAD_ID            , FE_SOLUTE_NATURAL_FLUX      , "Solute natural flux");
	REGISTER_FE_CLASS(FSMatchingOsmoticCoefficient, MODULE_MULTIPHASIC, FELOAD_ID            , FE_MATCHING_OSM_COEF        , "Matching osmotic coefficient");

#ifdef _DEBUG
	REGISTER_FE_CLASS(FSSBMPointSource, MODULE_MULTIPHASIC, FELOAD_ID, FE_SBM_POINT_SOURCE, "SBM point source");
#endif

	// --- FLUID MODULE ---
	REGISTER_FE_CLASS(FSFluidAnalysis               , MODULE_FLUID, FEANALYSIS_ID   , FE_STEP_FLUID                  , "Fluid Mechanics");
    REGISTER_FE_CLASS(FSFixedFluidVelocity          , MODULE_FLUID, FEBC_ID         , FE_FIXED_FLUID_VELOCITY        , "Zero fluid velocity");
    REGISTER_FE_CLASS(FSPrescribedFluidVelocity     , MODULE_FLUID, FEBC_ID         , FE_PRESCRIBED_FLUID_VELOCITY   , "Prescribed fluid velocity");
	REGISTER_FE_CLASS(FSFixedFluidDilatation        , MODULE_FLUID, FEBC_ID         , FE_FIXED_FLUID_DILATATION      , "Zero fluid dilatation");
	REGISTER_FE_CLASS(FSPrescribedFluidDilatation   , MODULE_FLUID, FEBC_ID         , FE_PRESCRIBED_FLUID_DILATATION , "Prescribed fluid dilatation");
	REGISTER_FE_CLASS(FSFluidTraction               , MODULE_FLUID, FELOAD_ID  , FE_FLUID_TRACTION              , "Fluid viscous traction");
    REGISTER_FE_CLASS(FSFluidPressureLoad           , MODULE_FLUID, FELOAD_ID  , FE_FLUID_PRESSURE_LOAD         , "Fluid pressure");
    REGISTER_FE_CLASS(FSFluidVelocity               , MODULE_FLUID, FELOAD_ID  , FE_FLUID_VELOCITY              , "Fluid velocity");
    REGISTER_FE_CLASS(FSFluidNormalVelocity         , MODULE_FLUID, FELOAD_ID  , FE_FLUID_NORMAL_VELOCITY       , "Fluid normal velocity");
    REGISTER_FE_CLASS(FSFluidRotationalVelocity     , MODULE_FLUID, FELOAD_ID  , FE_FLUID_ROTATIONAL_VELOCITY   , "Fluid rotational velocity");
    REGISTER_FE_CLASS(FSFluidFlowResistance         , MODULE_FLUID, FELOAD_ID  , FE_FLUID_FLOW_RESISTANCE       , "Fluid flow resistance");
    REGISTER_FE_CLASS(FSFluidFlowRCR                , MODULE_FLUID, FELOAD_ID  , FE_FLUID_FLOW_RCR              , "Fluid RCR");
    REGISTER_FE_CLASS(FSFluidBackflowStabilization  , MODULE_FLUID, FELOAD_ID  , FE_FLUID_BACKFLOW_STABIL       , "Fluid back flow stabilization");
    REGISTER_FE_CLASS(FSFluidTangentialStabilization, MODULE_FLUID, FELOAD_ID  , FE_FLUID_TANGENTIAL_STABIL     , "Fluid tangential stabilization");
    REGISTER_FE_CLASS(FSInitFluidDilatation         , MODULE_FLUID, FEIC_ID         , FE_INIT_FLUID_DILATATION       , "Fluid dilatation");

	REGISTER_FE_CLASS(FSNormalFlowSurface       , MODULE_FLUID, FENLCONSTRAINT_ID, FE_NORMAL_FLUID_FLOW           , "Normal flow constraint");
    REGISTER_FE_CLASS(FSFrictionlessFluidWall   , MODULE_FLUID, FENLCONSTRAINT_ID, FE_FRICTIONLESS_FLUID_WALL     , "Frictionless fluid wall");

    // --- FLUID-FSI MODULE ---
    REGISTER_FE_CLASS(FSFluidFSIAnalysis        , MODULE_FLUID_FSI, FEANALYSIS_ID   , FE_STEP_FLUID_FSI   , "Fluid-FSI Mechanics");
    REGISTER_FE_CLASS(FSFSITraction             , MODULE_FLUID_FSI, FELOAD_ID  , FE_FSI_TRACTION     , "FSI Interface Traction");
    REGISTER_FE_CLASS(FSBFSITraction            , MODULE_FLUID_FSI, FELOAD_ID  , FE_BFSI_TRACTION     , "Biphasic-FSI Interface Traction");
    

	// --- REACTION-DIFFUSION MODULE ---
	REGISTER_FE_CLASS(FSReactionDiffusionAnalysis, MODULE_REACTION_DIFFUSION, FEANALYSIS_ID   , FE_STEP_REACTION_DIFFUSION, "Reaction-Diffusion");
	REGISTER_FE_CLASS(FSConcentrationFlux        , MODULE_REACTION_DIFFUSION, FELOAD_ID  , FE_CONCENTRATION_FLUX, "Concentration Flux");

	// FEBio interface classes
	REGISTER_FE_CLASS(FEBioBoundaryCondition, 0, FEBC_ID               , FE_FEBIO_BC                , "[febio]");
	REGISTER_FE_CLASS(FEBioNodalLoad        , 0, FELOAD_ID        , FE_FEBIO_NODAL_LOAD        , "[febio]");
	REGISTER_FE_CLASS(FEBioSurfaceLoad      , 0, FELOAD_ID        , FE_FEBIO_SURFACE_LOAD      , "[febio]");
	REGISTER_FE_CLASS(FEBioBodyLoad         , 0, FELOAD_ID        , FE_FEBIO_BODY_LOAD         , "[febio]");
	REGISTER_FE_CLASS(FEBioInterface        , 0, FESURFACEINTERFACE_ID , FE_FEBIO_INTERFACE         , "[febio]");
	REGISTER_FE_CLASS(FEBioInitialCondition , 0, FEIC_ID               , FE_FEBIO_INITIAL_CONDITION , "[febio]");
	REGISTER_FE_CLASS(FEBioNLConstraint     , 0, FENLCONSTRAINT_ID     , FE_FEBIO_NLCONSTRAINT      , "[febio]");
	REGISTER_FE_CLASS(FEBioSurfaceConstraint, 0, FENLCONSTRAINT_ID     , FE_FEBIO_SURFACECONSTRAINT , "[febio]");
	REGISTER_FE_CLASS(FEBioBodyConstraint   , 0, FENLCONSTRAINT_ID     , FE_FEBIO_BODYCONSTRAINT    , "[febio]");
	REGISTER_FE_CLASS(FEBioAnalysisStep     , 0, FEANALYSIS_ID         , FE_STEP_FEBIO_ANALYSIS     , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidBC          , 0, FEBC_ID			   , FE_FEBIO_RIGID_BC          , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidIC          , 0, FEIC_ID			   , FE_FEBIO_RIGID_IC          , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidConnector   , 0, FENLCONSTRAINT_ID     , FE_FEBIO_RIGID_CONNECTOR   , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidLoad        , 0, FELOAD_ID             , FE_FEBIO_RIGID_LOAD        , "[febio]");
	REGISTER_FE_CLASS(FEBioMeshAdaptor      , 0, FEMESHADAPTOR_ID      , FE_FEBIO_MESH_ADAPTOR      , "[febio]");
	REGISTER_FE_CLASS(FEBioLoadController   , 0, FELOADCONTROLLER_ID   , FE_FEBIO_LOAD_CONTROLLER   , "[febio]");
	REGISTER_FE_CLASS(FEBioFunction1D       , 0, FEFUNCTION1D_ID       , FE_FEBIO_FUNCTION1D        , "[febio]");
	REGISTER_FE_CLASS(FEBioMaterialProperty , 0, FEMATERIALPROP_ID     , FE_FEBIO_MATERIAL_PROPERTY , "[febio]");
}

//-------------------------------------------------------------------------------------------------
void FSProject::SetUnits(int units)
{
	m_units = units;
}

//-------------------------------------------------------------------------------------------------
int FSProject::GetUnits() const
{
	return m_units;
}

//-------------------------------------------------------------------------------------------------
void FSProject::SetDefaultPlotVariables()
{
	int moduleId = GetModule();
	const char* szmod = FEBio::GetModuleName(moduleId); assert(szmod);
	if (szmod == nullptr) return;
	if (strcmp(szmod, "solid") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
	}
	else if (strcmp(szmod, "biphasic") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
		m_plt.AddPlotVariable("relative volume", true);
		m_plt.AddPlotVariable("solid stress", true);
		m_plt.AddPlotVariable("effective fluid pressure", true);
		m_plt.AddPlotVariable("fluid flux", true);
	}
	else if (strcmp(szmod, "heat") == 0)
	{
		m_plt.AddPlotVariable("temperature", true);
		m_plt.AddPlotVariable("heat flux", true);
	}
	else if ((strcmp(szmod, "multiphasic") == 0) || (strcmp(szmod, "solute") == 0))
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
		m_plt.AddPlotVariable("relative volume", true);
		m_plt.AddPlotVariable("solid stress", true);
		m_plt.AddPlotVariable("fluid flux", true);
		m_plt.AddPlotVariable("effective fluid pressure", true);
		m_plt.AddPlotVariable("effective solute concentration", true);
		m_plt.AddPlotVariable("solute concentration", true);
		m_plt.AddPlotVariable("solute flux", true);
	}
	else if (strcmp(szmod, "fluid") == 0)
	{
		m_plt.AddPlotVariable("fluid pressure", true);
		m_plt.AddPlotVariable("nodal fluid velocity", true);
		m_plt.AddPlotVariable("fluid stress", true);
		m_plt.AddPlotVariable("fluid velocity", true);
		m_plt.AddPlotVariable("fluid acceleration", true);
		m_plt.AddPlotVariable("fluid vorticity", true);
		m_plt.AddPlotVariable("fluid rate of deformation", true);
		m_plt.AddPlotVariable("fluid dilatation", true);
		m_plt.AddPlotVariable("fluid volume ratio", true);
	}
	else if (strcmp(szmod, "fluid-FSI") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("velocity", true);
		m_plt.AddPlotVariable("acceleration", true);
		m_plt.AddPlotVariable("fluid pressure", true);
		m_plt.AddPlotVariable("fluid stress", true);
		m_plt.AddPlotVariable("fluid velocity", true);
		m_plt.AddPlotVariable("fluid acceleration", true);
		m_plt.AddPlotVariable("fluid vorticity", true);
		m_plt.AddPlotVariable("fluid rate of deformation", true);
		m_plt.AddPlotVariable("fluid dilatation", true);
		m_plt.AddPlotVariable("fluid volume ratio", true);
	}
}

bool copyParameter(std::ostream& log, FSCoreBase* pc, const Param& p)
{
	// we try to copy the parameter value, but we need to make sure
	// that we don't change the type of pc parameter (unless it's a variable parameter). 
	Param* pi = pc->GetParam(p.GetShortName());
	if (pi)
	{
		if (pi->IsVariable() || p.IsVariable())
			*pi = p;
		else
		{
			if ((pi->GetParamType() == Param_INT) && (p.GetParamType() == Param_BOOL))
			{
				log << "warning: converting from bool to int (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
				pi->SetIntValue(p.GetBoolValue() ? 1 : 0);
			}
			else if ((pi->GetParamType() == Param_BOOL) && (p.GetParamType() == Param_INT))
			{
				log << "warning: converting from int to bool (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
				pi->SetBoolValue(p.GetIntValue() != 0 ? true : false);
			}
			else if ((pi->GetParamType() == Param_INT) && (p.GetParamType() == Param_FLOAT))
			{
				log << "warning: converting from float to int (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
				pi->SetIntValue((int)p.GetFloatValue());
			}
			else if ((pi->GetParamType() == Param_CHOICE) && (p.GetParamType() == Param_INT))
			{
#ifdef _DEBUG
				log << "warning: converting from int to choice (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
#endif
				pi->SetIntValue(p.GetIntValue());
			}
			else if ((pi->GetParamType() == Param_INT) && (p.GetParamType() == Param_CHOICE))
			{
#ifdef _DEBUG
				log << "warning: converting from choice to int (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
#endif
				pi->SetIntValue(p.GetIntValue());
			}
			else if ((p.GetParamType() == Param_VEC3D) && (pi->GetParamType() == Param_ARRAY_DOUBLE) && (pi->size() == 3))
			{
#ifdef _DEBUG
				log << "warning: converting from vec3d to double[3] (" << pc->GetName() << "." << pi->GetShortName() << ")\n";
#endif
				vec3d v = p.GetVec3dValue();
				std::vector<double> d = { v.x, v.y, v.z };
				pi->SetArrayDoubleValue(d);
			}
			else if (pi->GetParamType() != p.GetParamType())
			{
				return false;
			}
			else *pi = p;
		}

		// copy load curve IDs
		pi->SetLoadCurveID(p.GetLoadCurveID());
	}
	else { 
		return false; 
	}

	return true;
}

bool copyParameters(std::ostream& log, FSModelComponent* pd, const FSCoreBase* ps)
{
	bool bret = true;
	for (int i = 0; i < ps->Parameters(); ++i)
	{
		const Param& p = ps->GetParam(i);
		if (copyParameter(log, pd, p) == false)
		{
			bool bok = false;

			// vec3d parameters might be mapped to a vec3d valuator
			if (p.GetParamType() == Param_VEC3D)
			{
				FSProperty* prop = pd->FindProperty(p.GetShortName());
				if (prop && (prop->GetSuperClassID() == FEVEC3DVALUATOR_ID))
				{
					FSModelComponent* pc = FEBio::CreateClass(FEVEC3DVALUATOR_ID, "vector", pd->GetFSModel());
					prop->SetComponent(pc);

					Param* pv = pc->GetParam("vector");
					pv->SetVec3dValue(p.GetVec3dValue());

					int lc = p.GetLoadCurveID();
					pv->SetLoadCurveID(lc);

					bok = true;
				}
			}

			if (bok == false)
			{
				const char* sz = (p.GetShortName() ? p.GetShortName() : "(unnamed)");
				log << "Failed to copy parameter " << ps->GetName() << "." << sz << "\n";
				bret = bok;
			}
		}
	}
	if (bret == false)
	{
		string s = ps->GetName();
		if (s.empty()) s = "(no name; type = " + string(ps->GetTypeString()) + ")";
		log << "Errors encountered copying parameters from " << s << std::endl;
	}
	return bret;
}

void FSProject::ConvertToNewFormat(std::ostream& log)
{
	// although the active module was read in already, in previous versions of FEBio Studio
	// the module ID didn't really matter, so it's possible that it was not set properly. 
	// So, just to be sure, we're going to set the active module here as well, based on the first analysis' step type
	FSModel& fem = GetFSModel();
	if (fem.Steps() > 1)
	{
		FSAnalysisStep* step = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1)); assert(step);
		int ntype = step->GetType();
		switch (ntype)
		{
		case FE_STEP_MECHANICS      : FEBio::SetActiveModule("solid"); break;
		case FE_STEP_BIPHASIC       : FEBio::SetActiveModule("biphasic"); break;
		case FE_STEP_BIPHASIC_SOLUTE: FEBio::SetActiveModule("solute"); break;
		case FE_STEP_MULTIPHASIC    : FEBio::SetActiveModule("multiphasic"); break;
		case FE_STEP_FLUID          : FEBio::SetActiveModule("fluid"); break;
		case FE_STEP_FLUID_FSI      : FEBio::SetActiveModule("fluid-FSI"); break;
		default:
			assert(false);
		}
	}

	ConvertMaterials(log);
	ConvertSteps(log);
	ConvertDiscrete(log);
}

void copyModelComponent(std::ostream& log, FSModelComponent* pd, const FSModelComponent* ps)
{
	// first copy parameters
	copyParameters(log, pd, ps);

	// copy properties
	for (int i = 0; i < ps->Properties(); ++i)
	{
		const FSProperty& prop = ps->GetProperty(i);

		// see if prop has any allocated components
		int ncomp = 0;
		for (int j = 0; j < prop.Size(); ++j)
		{
			if (prop.GetComponent(j)) ncomp++;
		}
	
		if (ncomp > 0)
		{
			FSProperty* pi = pd->FindProperty(prop.GetName());
			if (pi)
			{
				int ncomp = prop.Size();
				pi->SetSize(ncomp);
				for (int j = 0; j < ncomp; ++j)
				{
					if (prop.GetComponent(j))
					{
						const FSModelComponent* pcj = dynamic_cast<const FSModelComponent*>(prop.GetComponent(j));
						if (pcj)
						{
							FSModel* fem = pd->GetFSModel();
							FSModelComponent* pcn = FEBio::CreateClass(pcj->GetSuperClassID(), pcj->GetTypeString(), fem);
							assert(pcn);

							if (dynamic_cast<const FSReactantMaterial*>(pcj))
							{
								const FSReactantMaterial* prm = dynamic_cast<const FSReactantMaterial*>(pcj);
								int v = prm->GetIntValue(FSReactionSpecies::MP_NU);
								int speciesType = prm->GetSpeciesType();
								int index = prm->GetIndex();
								const int solutes = fem->Solutes();
								if (speciesType == FSReactionSpecies::SBM_SPECIES) index += solutes;
								pcn->SetParamInt("vR", v);
								pcn->SetParamInt("species", index);
								pi->SetComponent(pcn, j);
							}
							else if (dynamic_cast<const FSProductMaterial*>(pcj))
							{
								const FSProductMaterial* ppm = dynamic_cast<const FSProductMaterial*>(pcj);
								int v = ppm->GetIntValue(FSReactionSpecies::MP_NU);
								int speciesType = ppm->GetSpeciesType();
								int index = ppm->GetIndex();
								const int solutes = fem->Solutes();
								if (speciesType == FSReactionSpecies::SBM_SPECIES) index += solutes;
								pcn->SetParamInt("vP", v);
								pcn->SetParamInt("species", index);
								pi->SetComponent(pcn, j);
							}
							else if (pcn)
							{
								copyModelComponent(log, pcn, pcj);
								pi->SetComponent(pcn, j);
							}
							else
							{
								log << "Failed to create model component!\n";
							}
						}
						else
						{
							log << "dynamic_cast to FSModelComponent failed!\n";
						}
					}
				}
			}
			else
			{
				log << "Failed to find property " << prop.GetName() << std::endl;
			}
		}
	}
}

void copyRigidMaterial(std::ostream& log, FSRigidMaterial* prm, FSMaterial* febMat)
{
	copyParameter(log, febMat, prm->GetParam(FSRigidMaterial::MP_DENSITY));
	copyParameter(log, febMat, prm->GetParam(FSRigidMaterial::MP_E));
	copyParameter(log, febMat, prm->GetParam(FSRigidMaterial::MP_V));
	copyParameter(log, febMat, prm->GetParam(FSRigidMaterial::MP_RC));

	bool autoCom = prm->GetBoolValue(FSRigidMaterial::MP_COM);
	febMat->SetParamBool("override_com", !autoCom);
}

void FSProject::ConvertMaterials(std::ostream& log)
{
	FSModel& fem = GetFSModel();

	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);

		FSMaterial* pm = mat->GetMaterialProperties();
		if (pm == nullptr)
		{
			log << "ERROR: Material \"" << mat->GetName() << "\" has no properties!" << std::endl;
		}
		else
		{
			FSMaterial* febMat = FEBio::CreateMaterial(pm->GetTypeString(), &fem);
			if (febMat == nullptr)
			{
				log << "Failed to create FEBio material " << pm->GetTypeString() << std::endl;
			}
			else
			{
				if (dynamic_cast<FSRigidMaterial*>(pm))
					copyRigidMaterial(log, dynamic_cast<FSRigidMaterial*>(pm), febMat);
				else
					copyModelComponent(log, febMat, pm);

				if (pm->HasMaterialAxes())
				{
					// see if the febio material has the mat_axis property defined. 
					FSProperty* matAxis = febMat->FindProperty("mat_axis");
					if (matAxis)
					{
						FSAxisMaterial* axis = pm->m_axes;
						switch (axis->m_naopt)
						{
						case FE_AXES_LOCAL: 
						{
							FSModelComponent* febAxis = FEBio::CreateClass(FEMAT3DVALUATOR_ID, "local", &fem); assert(febAxis);
							if (febAxis)
							{
								std::vector<int> n = { axis->m_n[0], axis->m_n[1], axis->m_n[2] };
								febAxis->GetParam("local")->SetArrayIntValue(n);
								matAxis->SetComponent(febAxis);
							}
						}
						break;
						case FE_AXES_VECTOR: 
						{
							FSModelComponent* febAxis = FEBio::CreateClass(FEMAT3DVALUATOR_ID, "vector", &fem); assert(febAxis);
							if (febAxis)
							{
								febAxis->SetParamVec3d("a", axis->m_a);
								febAxis->SetParamVec3d("d", axis->m_d);
								matAxis->SetComponent(febAxis);
							}
						}
						break;
						case FE_AXES_ANGLES: 
						{
							FSModelComponent* febAxis = FEBio::CreateClass(FEMAT3DVALUATOR_ID, "vector", &fem); assert(febAxis);
							if (febAxis)
							{
								febAxis->SetParamFloat("theta", axis->m_theta);
								febAxis->SetParamFloat("phi"  , axis->m_phi);
								matAxis->SetComponent(febAxis);
							}
						}
						break;
						case FE_AXES_CYLINDRICAL: 
						{
							FSModelComponent* febAxis = FEBio::CreateClass(FEMAT3DVALUATOR_ID, "cylindrical", &fem); assert(febAxis);
							if (febAxis)
							{
								febAxis->SetParamVec3d("center", axis->m_center);
								febAxis->SetParamVec3d("axis"  , axis->m_axis);
								febAxis->SetParamVec3d("vector", axis->m_vec);
								matAxis->SetComponent(febAxis);
							}
						}
						break;
						case FE_AXES_SPHERICAL: 
						{
							FSModelComponent* febAxis = FEBio::CreateClass(FEMAT3DVALUATOR_ID, "spherical", &fem); assert(febAxis);
							if (febAxis)
							{
								febAxis->SetParamVec3d("center", axis->m_center);
								febAxis->SetParamVec3d("vector", axis->m_vec);
								matAxis->SetComponent(febAxis);
							}
						}
						break;
						default:
							log << "Unkown mat axis type in " << mat->GetName() << std::endl;
						}
					}
					else
					{
						log << "Can't map mat_axis property of " << mat->GetName() << std::endl;
					}
				}

				mat->SetMaterialProperties(febMat);
			}
		}
	}
}

bool FSProject::ConvertSteps(std::ostream& log)
{
	FSModel& fem = GetFSModel();

	// loop over all steps (skipping the initial step)
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* oldStep = fem.GetStep(i);
		FSStep* newStep = nullptr;
		if (oldStep->GetType() == FE_STEP_INITIAL)
		{
			newStep = new FSInitialStep(&fem);
		}
		else
		{
			FSAnalysisStep* step = dynamic_cast<FSAnalysisStep*>(fem.GetStep(i)); assert(step);
			if (step == nullptr) return false;

			const char* stepType = FEBio::GetActiveModuleName();
			FEBioAnalysisStep* febioStep = dynamic_cast<FEBioAnalysisStep*>(FEBio::CreateStep(stepType, &fem)); assert(febioStep);
			if (febioStep == nullptr)
			{
				log << "Failed to create FEBio step.\n";
				return false;
			}
			FEBio::InitDefaultProps(febioStep);

			ConvertStepSettings(log, *febioStep, *step);

			newStep = febioStep;
		}

		// copy the step name and ID
		newStep->SetName(oldStep->GetName());
		newStep->SetID(oldStep->GetID());

		// copy settings
		ConvertStep(log, *newStep, *oldStep);

		// swap old with new step
		fem.ReplaceStep(i, newStep);
		delete oldStep;
	}

	return true;
}

void FSProject::ConvertStep(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	// convert the model components
	ConvertStepBCs             (log, newStep, oldStep);
	ConvertStepLoads           (log, newStep, oldStep);
	ConvertStepContact         (log, newStep, oldStep);
	ConvertStepConstraints     (log, newStep, oldStep);
	ConvertStepRigidConstraints(log, newStep, oldStep);
	ConvertStepRigidConnectors (log, newStep, oldStep);
	ConvertLinearConstraints   (log, newStep, oldStep);
}

void FSProject::ConvertStepRigidConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();
	for (int i = 0; i < oldStep.RigidConstraints(); ++i)
	{
		FSRigidConstraint* pc = oldStep.RigidConstraint(i);

		if (pc->Type() == FE_RIGID_FIXED)
		{
			FSRigidFixed* pf = dynamic_cast<FSRigidFixed*>(pc); assert(pf);

			FSRigidBC* pcfeb = FEBio::CreateRigidBC("rigid_fixed", fem); assert(pcfeb);
			pcfeb->SetParamBool("Rx_dof", pf->GetBoolValue(0));
			pcfeb->SetParamBool("Ry_dof", pf->GetBoolValue(1));
			pcfeb->SetParamBool("Rz_dof", pf->GetBoolValue(2));
			pcfeb->SetParamBool("Ru_dof", pf->GetBoolValue(3));
			pcfeb->SetParamBool("Rv_dof", pf->GetBoolValue(4));
			pcfeb->SetParamBool("Rw_dof", pf->GetBoolValue(5));

			// copy the name 
			pcfeb->SetName(pc->GetName());

			// copy rigid body
			pcfeb->SetMaterialID(pc->GetMaterialID());

			newStep.AddRigidBC(pcfeb);
		}
		else if (pc->Type() == FE_RIGID_FORCE)
		{
			FSRigidForce* pf = dynamic_cast<FSRigidForce*>(pc);

			int dof = pf->GetDOF();
			if (dof < 3)
			{
				FSRigidLoad* pl = FEBio::CreateRigidLoad("rigid_force", fem);
				copyParameters(log, pl, pc);
				pl->SetMaterialID(pc->GetMaterialID());
				pl->SetName(pc->GetName());
				newStep.AddRigidLoad(pl);
			}
			else
			{
				FSRigidLoad* pl = FEBio::CreateRigidLoad("rigid_moment", fem);
				copyParameters(log, pl, pc);
				pl->SetParamInt("dof", dof - 3); // NOTE: This is necessary since dof is zero-based!
				pl->SetMaterialID(pc->GetMaterialID());
				pl->SetName(pc->GetName());
				newStep.AddRigidLoad(pl);
			}
		}
		else if (pc->Type() == FE_RIGID_DISPLACEMENT)
		{
			FSRigidPrescribed* pf = dynamic_cast<FSRigidPrescribed*>(pc); assert(pf);
			int bc = pf->GetDOF();
			if (bc < 3)
			{
				FSRigidBC* pcfeb = FEBio::CreateRigidBC("rigid_displacement", fem); assert(pcfeb);
				copyParameters(log, pcfeb, pc);
				pcfeb->SetName(pc->GetName());
				pcfeb->SetMaterialID(pc->GetMaterialID());
				newStep.AddRigidBC(pcfeb);
			}
			else
			{
				FSRigidBC* pcfeb = FEBio::CreateRigidBC("rigid_rotation", fem); assert(pcfeb);
				copyParameters(log, pcfeb, pc);
				pcfeb->SetParamInt("dof", bc - 3); // NOTE: This is necessary since dof is zero-based!
				pcfeb->SetName(pc->GetName());
				pcfeb->SetMaterialID(pc->GetMaterialID());
				newStep.AddRigidBC(pcfeb);
			}
		}
		else if (pc->Type() == FE_RIGID_INIT_VELOCITY)
		{
			FSRigidVelocity* pf = dynamic_cast<FSRigidVelocity*>(pc); assert(pf);
			FSRigidIC* pcfeb = FEBio::CreateRigidIC("initial_rigid_velocity", fem); assert(pcfeb);
			copyParameters(log, pcfeb, pc);
			pcfeb->SetName(pc->GetName());
			pcfeb->SetMaterialID(pc->GetMaterialID());
			newStep.AddRigidIC(pcfeb);
		}
		else if (pc->Type() == FE_RIGID_INIT_ANG_VELOCITY)
		{
			FSRigidAngularVelocity* pf = dynamic_cast<FSRigidAngularVelocity*>(pc); assert(pf);
			FSRigidIC* pcfeb = FEBio::CreateRigidIC("initial_rigid_angular_velocity", fem); assert(pcfeb);
			copyParameters(log, pcfeb, pc);
			pcfeb->SetName(pc->GetName());
			pcfeb->SetMaterialID(pc->GetMaterialID());
			newStep.AddRigidIC(pcfeb);
		}
		else
		{
			assert(false);
		}
	}

	// Rigid cables are read in directly as FEBio loads, so we just need to clone them.
	for (int i = 0; i < oldStep.RigidLoads(); ++i)
	{
		FSRigidLoad* plo = oldStep.RigidLoad(i);
		FSRigidLoad* pln = FEBio::CreateRigidLoad(plo->GetTypeString(), fem);
		copyModelComponent(log, pln, plo);
		pln->SetName(plo->GetName());
		newStep.AddRigidLoad(pln);
	}
}

void FSProject::ConvertStepRigidConnectors(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();
	for (int i = 0; i < oldStep.RigidConnectors(); ++i)
	{
		FSRigidConnector* pc = oldStep.RigidConnector(i);
		FSRigidConnector* pcfeb = nullptr;
		switch (pc->Type())
		{
		case FE_RC_SPHERICAL_JOINT   : pcfeb = FEBio::CreateRigidConnector("rigid spherical joint"  , fem); break;
		case FE_RC_REVOLUTE_JOINT	 : pcfeb = FEBio::CreateRigidConnector("rigid revolute joint"   , fem); break;
		case FE_RC_PRISMATIC_JOINT	 : pcfeb = FEBio::CreateRigidConnector("rigid prismatic joint"  , fem); break;
		case FE_RC_CYLINDRICAL_JOINT : pcfeb = FEBio::CreateRigidConnector("rigid cylindrical joint", fem); break;
		case FE_RC_PLANAR_JOINT      : pcfeb = FEBio::CreateRigidConnector("rigid planar joint"     , fem); break;
		case FE_RC_SPRING            : pcfeb = FEBio::CreateRigidConnector("rigid spring"           , fem); break;
		case FE_RC_DAMPER            : pcfeb = FEBio::CreateRigidConnector("rigid damper"           , fem); break;
		case FE_RC_ANGULAR_DAMPER	 : pcfeb = FEBio::CreateRigidConnector("rigid angular damper"   , fem); break;
		case FE_RC_CONTRACTILE_FORCE : pcfeb = FEBio::CreateRigidConnector("rigid contractile force", fem); break;
		case FE_RC_RIGID_LOCK        : pcfeb = FEBio::CreateRigidConnector("rigid lock"             , fem); break;
		case FE_RC_GENERIC_JOINT	 : pcfeb = FEBio::CreateRigidConnector("generic rigid joint"    , fem); break;
		default:
			assert(false);
		}
		assert(pcfeb);
		
		if (pcfeb)
		{
			// copy the name 
			pcfeb->SetName(pc->GetName());

			// copy rigid bodys
			pcfeb->SetRigidBody1(pc->GetRigidBody1());
			pcfeb->SetRigidBody2(pc->GetRigidBody2());

			// replace parameters
			copyParameters(log, pcfeb, pc);

			newStep.AddRigidConnector(pcfeb);
		}
	}
}

void FSProject::ConvertStepConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();
	for (int i = 0; i < oldStep.Constraints(); ++i)
	{
		FSModelConstraint* pc = oldStep.Constraint(i);
		FSModelConstraint* pcfeb = nullptr;
		switch (pc->Type())
		{
		case FE_SYMMETRY_PLANE          : pcfeb = FEBio::CreateModelConstraint("symmetry plane", fem); break;
		case FE_NORMAL_FLUID_FLOW       : pcfeb = FEBio::CreateModelConstraint("normal fluid flow", fem); break;
		case FE_VOLUME_CONSTRAINT       : pcfeb = FEBio::CreateModelConstraint("volume", fem); break;
//		case FE_WARP_CONSTRAINT         : pcfeb = FEBio::CreateModelConstraint("", fem); break;
		case FE_FRICTIONLESS_FLUID_WALL : pcfeb = FEBio::CreateModelConstraint("frictionless fluid wall", fem); break;
		case FE_INSITUSTRETCH_CONSTRAINT: pcfeb = FEBio::CreateModelConstraint("in-situ stretch", fem); break;
		case FE_PRESTRAIN_CONSTRAINT    : pcfeb = FEBio::CreateModelConstraint("prestrain", fem); break;
		default:
			assert(false);
		}
		assert(pcfeb);

		if (pcfeb)
		{
			// copy the name 
			pcfeb->SetName(pc->GetName());

			// steal the item list
			pcfeb->SetItemList(pc->GetItemList());
			pc->SetItemList(nullptr);

			// replace parameters
			copyParameters(log, pcfeb, pc);

			newStep.AddConstraint(pcfeb);
		}
		else
		{
			log << "Failed to convert constraint " << pc->GetName() << std::endl;
		}
	}
}

void FSProject::ConvertStepContact(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();
	for (int i = 0; i < oldStep.Interfaces(); ++i)
	{
		FSInterface* pi = oldStep.Interface(i);
		FSInterface* newpi = nullptr;

		// some special cases
		if (pi->Type() == FE_RIGID_WALL)
		{
			FSSoloInterface* pc = dynamic_cast<FSSoloInterface*>(pi); assert(pc);
			FSSurfaceConstraint* newpc = FEBio::CreateSurfaceConstraint("rigid_wall", fem);

			newpc->SetItemList(pc->GetItemList());
			pc->SetItemList(nullptr);

			newpc->SetName(pc->GetName());

			// replace parameters
			std::vector<double> plane(4, 0);
			for (int i = 0; i < pc->Parameters(); ++i)
			{
				Param& p = pc->GetParam(i);
				if      (strcmp(p.GetShortName(), "a") == 0) plane[0] = p.GetFloatValue();
				else if (strcmp(p.GetShortName(), "b") == 0) plane[1] = p.GetFloatValue();
				else if (strcmp(p.GetShortName(), "c") == 0) plane[2] = p.GetFloatValue();
				else if (strcmp(p.GetShortName(), "d") == 0) plane[3] = p.GetFloatValue();
				else copyParameter(log, newpc, p);
			}

			newpc->GetParam("plane")->SetArrayDoubleValue(plane);

			newStep.AddConstraint(newpc);
		}
		else if (pi->Type() == FE_RIGID_INTERFACE)
		{
			// rigid interface became a boundary condition
			FSRigidInterface* pr = dynamic_cast<FSRigidInterface*>(pi);
			FSBoundaryCondition* pbc = FEBio::CreateBoundaryCondition("rigid", fem);

			pbc->SetParamInt("rb", pr->GetRigidBody()->GetID());

			pbc->SetItemList(pr->GetItemList());
			pr->SetItemList(nullptr);

			pbc->SetName(pr->GetName());

			newStep.AddBC(pbc);
		}
		else
		{
			switch (pi->Type())
			{
	//		case FE_RIGID_INTERFACE				: newpi = FEBio::CreatePairedInterface("rigid", fem); break;
			case FE_SLIDING_INTERFACE           : newpi = FEBio::CreatePairedInterface("sliding-elastic", fem); break;
	//		case FE_RIGID_JOINT					: newpi = FEBio::CreatePairedInterface(, fem); break;
			case FE_TIED_INTERFACE              : newpi = FEBio::CreatePairedInterface("tied-node-on-facet", fem); break;
			case FE_PORO_INTERFACE              : newpi = FEBio::CreatePairedInterface("sliding-biphasic", fem); break;
			case FE_PORO_SOLUTE_INTERFACE		: newpi = FEBio::CreatePairedInterface("sliding-biphasic-solute", fem); break;
			case FE_TENSCOMP_INTERFACE          : newpi = FEBio::CreatePairedInterface("sliding-elastic", fem); break;
			case FE_TIEDBIPHASIC_INTERFACE      : newpi = FEBio::CreatePairedInterface("tied-biphasic", fem); break;
	//		case FE_SPRINGTIED_INTERFACE		: newpi = FEBio::CreatePairedInterface(, fem); break;
			case FE_MULTIPHASIC_INTERFACE       : newpi = FEBio::CreatePairedInterface("sliding-multiphasic", fem); break;
			case FE_STICKY_INTERFACE            : newpi = FEBio::CreatePairedInterface("sticky", fem); break;
			case FE_PERIODIC_BOUNDARY			: newpi = FEBio::CreatePairedInterface("periodic boundary", fem); break;
	//		case FE_RIGID_SPHERE_CONTACT		: newpi = FEBio::CreatePairedInterface(, fem); break;
			case FE_SLIDING_WITH_GAPS           : newpi = FEBio::CreatePairedInterface("sliding-node-on-facet", fem); break;
			case FE_FACET_ON_FACET_SLIDING      : newpi = FEBio::CreatePairedInterface("sliding-facet-on-facet", fem); break;
			case FE_FACET_ON_FACET_TIED         : newpi = FEBio::CreatePairedInterface("tied-facet-on-facet", fem); break;
			case FE_TIEDMULTIPHASIC_INTERFACE   : newpi = FEBio::CreatePairedInterface("tied-multiphasic", fem); break;
			case FE_TIED_ELASTIC_INTERFACE      : newpi = FEBio::CreatePairedInterface("tied-elastic", fem); break;
	//		case FE_GAPHEATFLUX_INTERFACE		: newpi = FEBio::CreatePairedInterface(, fem); break;
			case FE_CONTACTPOTENTIAL_CONTACT    : newpi = FEBio::CreatePairedInterface("contact potential", fem); break;
			default:
				assert(false);
			}
			assert(newpi);

			if (newpi)
			{
				// copy the name 
				newpi->SetName(pi->GetName());

				// steal the item lists
				if (dynamic_cast<FSPairedInterface*>(pi))
				{
					FSPairedInterface* pc_old = dynamic_cast<FSPairedInterface*>(pi);
					FSPairedInterface* pc_new = dynamic_cast<FSPairedInterface*>(newpi); assert(pc_new);

					pc_new->SetPrimarySurface(pc_old->GetPrimarySurface()); pc_old->SetPrimarySurface(nullptr);
					pc_new->SetSecondarySurface(pc_old->GetSecondarySurface()); pc_old->SetSecondarySurface(nullptr);
				}
				if (dynamic_cast<FSSoloInterface*>(pi))
				{
					FSSoloInterface* pc_old = dynamic_cast<FSSoloInterface*>(pi);
					FSSoloInterface* pc_new = dynamic_cast<FSSoloInterface*>(newpi); assert(pc_new);

					pc_new->SetItemList(pc_old->GetItemList()); pc_old->SetItemList(nullptr);
				}

				// replace parameters
				bool bret = true;
				for (int i = 0; i < pi->Parameters(); ++i)
				{
					Param& p = pi->GetParam(i);

					// regrettably, it was missed that the minaug and maxaug were defined as doubles. 
					if (strcmp(p.GetShortName(), "minaug") == 0)
					{
						Param* ppi = newpi->GetParam("minaug");
						ppi->SetIntValue((int)p.GetFloatValue());
					}
					else if (strcmp(p.GetShortName(), "maxaug") == 0)
					{
						Param* ppi = newpi->GetParam("maxaug");
						ppi->SetIntValue((int)p.GetFloatValue());
					}
					else if (copyParameter(log, newpi, p) == false) bret = false;
				}
				if (bret == false) log << "Errors encountered copying parameters from " << pi->GetName() << std::endl;

				newStep.AddInterface(newpi);
			}
			else
			{
				log << "Failed to convert interface " << pi->GetName() << std::endl;
			}
		}
	}
}

void FSProject::ConvertStepLoads(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();
	for (int i = 0; i < oldStep.Loads(); ++i)
	{
		FSLoad* pl = oldStep.Load(i);
		FSLoad* febLoad = nullptr;
		if (pl->Type() == FE_NODAL_DOF_LOAD)
		{
			FSNodalDOFLoad* pnl = dynamic_cast<FSNodalDOFLoad*>(pl);

			febLoad = FEBio::CreateNodalLoad("nodal_force", fem);
			int bc = pnl->GetDOF();
			double s = pnl->GetLoad();

			vec3d f;
			switch (bc)
			{
			case 0: f = vec3d(s, 0, 0); break;
			case 1: f = vec3d(0, s, 0); break;
			case 2: f = vec3d(0, 0, s); break;
			}

			Param* pf = febLoad->GetParam("value"); assert(pf);
			pf->SetVec3dValue(f);

			int lc = pnl->GetParam(FSNodalDOFLoad::LOAD).GetLoadCurveID();
			pf->SetLoadCurveID(lc);
		}
		else if (pl->Type() == FE_FLUID_ROTATIONAL_VELOCITY)
		{
			// NOTE: This is now a boundary condition!
			FSBoundaryCondition* bcfeb = FEBio::CreateBoundaryCondition("fluid rotational velocity", fem); assert(bcfeb);

			// replace parameters
			copyParameters(log, bcfeb, pl);

			// copy the name 
			bcfeb->SetName(pl->GetName());

			// steal the item list
			bcfeb->SetItemList(pl->GetItemList());
			pl->SetItemList(nullptr);

			newStep.AddBC(bcfeb);

			// let's move on
			continue;
		}
		else
		{
			if (dynamic_cast<FSSurfaceLoad*>(pl))
			{
				febLoad = FEBio::CreateSurfaceLoad(pl->GetTypeString(), fem);
			}
			else if (dynamic_cast<FSBodyLoad*>(pl))
			{
				febLoad = FEBio::CreateBodyLoad(pl->GetTypeString(), fem);
			}
			assert(febLoad);

			// replace parameters
			copyParameters(log, febLoad, pl);
		}

		if (febLoad)
		{
			// copy the name 
			febLoad->SetName(pl->GetName());

			// steal the item list
			febLoad->SetItemList(pl->GetItemList());
			pl->SetItemList(nullptr);

			newStep.AddLoad(febLoad);
		}
		else
		{
			log << "Failed to convert load " << pl->GetName() << std::endl;
		}
	}
}

void FSProject::ConvertStepICs(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();

	// convert all model components
	for (int i = 0; i < oldStep.ICs(); ++i)
	{
		FSInitialCondition* ic = oldStep.IC(i);
		FSInitialCondition* febic = nullptr;

		switch (ic->Type())
		{
		case FE_INIT_NODAL_VELOCITIES      : febic = FEBio::CreateInitialCondition("velocity", fem); break;
//		case FE_INIT_NODAL_SHELL_VELOCITIES: break;
		case FE_INIT_FLUID_PRESSURE        : febic = FEBio::CreateInitialCondition("initial fluid pressure", fem); break;
//		case FE_INIT_SHELL_FLUID_PRESSURE  : break;
		case FE_INIT_CONCENTRATION         : febic = FEBio::CreateInitialCondition("initial concentration", fem); break;
//		case FE_INIT_SHELL_CONCENTRATION   : break;
//		case FE_INIT_TEMPERATURE           : break;
		case FE_INIT_FLUID_DILATATION      : febic = FEBio::CreateInitialCondition("initial fluid dilatation", fem); break;
		case FE_INIT_PRESTRAIN             : febic = FEBio::CreateInitialCondition("prestrain", fem); break;
		default:
			assert(false);
		}
		assert(febic);

		if (febic)
		{
			febic->SetName(ic->GetName());
			febic->SetItemList(ic->GetItemList());
			ic->SetItemList(nullptr);

			copyParameters(log, febic, ic);

			newStep.AddIC(febic);
		}
	}
}


void FSProject::ConvertStepBCs(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = newStep.GetFSModel();

	// convert all model components
	for (int i = 0; i < oldStep.BCs(); ++i)
	{
		FSBoundaryCondition* pb = oldStep.BC(i);
		FSBoundaryCondition* febbc = nullptr;
		FSFixedDOF* pbc = dynamic_cast<FSFixedDOF*>(pb);
		if (pbc)
		{
			switch (pbc->Type())
			{
			case FE_FIXED_DISPLACEMENT      : febbc = FEBio::CreateBoundaryCondition("zero displacement"      , fem); break;
			case FE_FIXED_ROTATION          : febbc = FEBio::CreateBoundaryCondition("zero rotation"          , fem); break;
			case FE_FIXED_SHELL_DISPLACEMENT: febbc = FEBio::CreateBoundaryCondition("zero shell displacement", fem); break;
			case FE_FIXED_FLUID_PRESSURE    : febbc = FEBio::CreateBoundaryCondition("zero fluid pressure"    , fem); break;
			case FE_FIXED_TEMPERATURE       : febbc = FEBio::CreateBoundaryCondition("zero temperature"       , fem); break;
			case FE_FIXED_CONCENTRATION     : febbc = FEBio::CreateBoundaryCondition("zero concentration"     , fem); break;
			case FE_FIXED_FLUID_VELOCITY    : febbc = FEBio::CreateBoundaryCondition("zero fluid velocity"    , fem); break;
			case FE_FIXED_FLUID_DILATATION  : febbc = FEBio::CreateBoundaryCondition("zero fluid velocity"    , fem); break;
			default:
				assert(false);
			}
			assert(febbc);

			if (febbc)
			{
				// TODO: Add support for concentrations
				int bc = pbc->GetBC();
				switch (pbc->Type())
				{
				case FE_FIXED_DISPLACEMENT:
					if (bc & 1) febbc->SetParamBool("x_dof", true);
					if (bc & 2) febbc->SetParamBool("y_dof", true);
					if (bc & 4) febbc->SetParamBool("z_dof", true);
					break;
				case FE_FIXED_ROTATION:
					if (bc & 1) febbc->SetParamBool("u_dof", true);
					if (bc & 2) febbc->SetParamBool("v_dof", true);
					if (bc & 4) febbc->SetParamBool("w_dof", true);
					break;
				case FE_FIXED_SHELL_DISPLACEMENT:
					if (bc & 1) febbc->SetParamBool("sx_dof", true);
					if (bc & 2) febbc->SetParamBool("sy_dof", true);
					if (bc & 4) febbc->SetParamBool("sz_dof", true);
					break;
				case FE_FIXED_FLUID_VELOCITY:
					if (bc & 1) febbc->SetParamBool("wx_dof", true);
					if (bc & 2) febbc->SetParamBool("wy_dof", true);
					if (bc & 4) febbc->SetParamBool("wz_dof", true);
					break;
				case FE_FIXED_FLUID_PRESSURE:
				case FE_FIXED_FLUID_DILATATION:
				case FE_FIXED_TEMPERATURE:
					// No need to do anything
					break;
				case FE_FIXED_CONCENTRATION:
					febbc->SetParamInt("c_dof", bc - 1);
					break;
				default:
					log << "Unable to map degrees of freedom for " << pb->GetName() << std::endl;
				}

				// copy the name 
				febbc->SetName(pbc->GetName());

				// steal the item list
				febbc->SetItemList(pbc->GetItemList());
				pbc->SetItemList(nullptr);

				newStep.AddBC(febbc);
			}
			else
			{
				log << "Unable to convert boundary condition " << pb->GetName() << std::endl;
			}
		}

		FSPrescribedDOF* pdc = dynamic_cast<FSPrescribedDOF*>(pb);
		if (pdc)
		{
			switch (pdc->Type())
			{
			case FE_PRESCRIBED_DISPLACEMENT      : febbc = FEBio::CreateBoundaryCondition("prescribed displacement", fem); break;
			case FE_PRESCRIBED_ROTATION          : febbc = FEBio::CreateBoundaryCondition("prescribed rotation", fem); break;
			case FE_PRESCRIBED_SHELL_DISPLACEMENT: febbc = FEBio::CreateBoundaryCondition("prescribed shell displacement", fem); break;
			case FE_PRESCRIBED_FLUID_PRESSURE    : febbc = FEBio::CreateBoundaryCondition("prescribed fluid pressure", fem); break;
			case FE_PRESCRIBED_TEMPERATURE       : febbc = FEBio::CreateBoundaryCondition("prescribed temperature", fem); break;
			case FE_PRESCRIBED_CONCENTRATION     : febbc = FEBio::CreateBoundaryCondition("prescribed concentration", fem); break;
			case FE_PRESCRIBED_FLUID_VELOCITY    : febbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", fem); break;
			case FE_PRESCRIBED_FLUID_DILATATION  : febbc = FEBio::CreateBoundaryCondition("prescribed fluid dilatation", fem); break;
			default:
				assert(false);
			}
			assert(febbc);

			if (febbc)
			{
				int dof = pdc->GetDOF();
				bool brel = pdc->GetRelativeFlag();

				if (febbc->GetParam("dof")) febbc->SetParamInt("dof", dof);
				febbc->SetParamBool("relative", brel);

				Param& scl = *pdc->GetParam("scale");
				Param& val = *febbc->GetParam("value");
				val = scl;

				// copy the name 
				febbc->SetName(pdc->GetName());

				// steal the item list
				febbc->SetItemList(pdc->GetItemList());
				pdc->SetItemList(nullptr);

				newStep.AddBC(febbc);
			}
			else
			{
				log << "Unable to convert boundary condition " << pb->GetName() << std::endl;
			}
		}
	}
}

void FSProject::ConvertLinearConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep)
{
	FSModel* fem = oldStep.GetFSModel();
	for (int i = 0; i < oldStep.LinearConstraints(); ++i)
	{
		FSLinearConstraintSet& lc = *oldStep.LinearConstraint(i);

		FSModelConstraint* flc = FEBio::CreateNLConstraint("linear constraint", fem);
		flc->SetParamFloat("tol"    , lc.m_atol   );
		flc->SetParamFloat("penalty", lc.m_penalty);
		flc->SetParamInt  ("maxaug" , lc.m_nmaxaug);

		FSProperty& lcp = *flc->FindProperty("linear_constraint");
		for (int j = 0; j < lc.m_set.size(); ++j)
		{
			FSLinearConstraintSet::LinearConstraint& lcj = lc.m_set[j];
			FSGenericClass* pc = FEBio::CreateGenericClass("linear_constraint", fem);

			FSProperty* node = pc->FindProperty("node");
			for (int k = 0; k < lcj.m_dof.size(); ++k)
			{
				FSLinearConstraintSet::LinearConstraint::DOF& dof = lcj.m_dof[k];
				FSGenericClass* pn = FEBio::CreateGenericClass("node", fem);
				pn->SetParamInt("id", dof.node);
				pn->SetParamInt("bc", dof.bc);
				pn->SetParamFloat("node", dof.s);

				node->AddComponent(pn);
			}
			lcp.AddComponent(pc);
		}

		newStep.AddConstraint(flc);
	}
}

void FSProject::ConvertStepSettings(std::ostream& log, FEBioAnalysisStep& febStep, FSAnalysisStep& oldStep)
{
	STEP_SETTINGS& ops = oldStep.GetSettings();

	febStep.SetParamInt("analysis", ops.nanalysis);
	febStep.SetParamInt("time_steps", ops.ntime);
	febStep.SetParamFloat("step_size", ops.dt);

	febStep.SetParamInt("plot_level", ops.plot_level);
	febStep.SetParamInt("plot_stride", ops.plot_stride);

	// auto time stepper settings
	FSProperty* timeStepperProp = febStep.FindProperty("time_stepper");
	FSCoreBase* timeStepper = timeStepperProp->GetComponent(0);
	if (ops.bauto && timeStepper)
	{
		timeStepper->SetParamInt("max_retries", ops.mxback);
		timeStepper->SetParamInt("opt_iter", ops.iteopt);
		timeStepper->SetParamFloat("dtmin", ops.dtmin);
		timeStepper->SetParamFloat("dtmax", ops.dtmax);
		timeStepper->SetParamInt("aggressiveness", ops.ncut);
	}
	else timeStepperProp->SetComponent(nullptr);

	// solver settings
	FSProperty* solverProp = febStep.FindProperty("solver");
	FSCoreBase* solver = solverProp->GetComponent(0);
	if (solver)
	{
		solver->SetParamInt("max_refs", ops.maxref);
		solver->SetParamBool("diverge_reform", ops.bdivref);
		solver->SetParamBool("reform_each_time_step", ops.brefstep);
		solver->SetParamInt("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		solver->SetParamInt("equation_scheme", ops.neqscheme);

		FSProperty* qnProp = solver->FindProperty("qn_method");
		qnProp->SetComponent(nullptr);
		FSCoreBase* qnSolver = nullptr;

		for (int i = 0; i < oldStep.Parameters(); ++i)
		{
			Param& p = oldStep.GetParam(i);

			if (strcmp(p.GetShortName(), "qnmethod") == 0)
			{
				int qnmethod = p.GetIntValue();
				switch (qnmethod)
				{
				case 0: qnSolver = FEBio::CreateClass(FENEWTONSTRATEGY_ID, "BFGS", febStep.GetFSModel()); break;
				case 1: qnSolver = FEBio::CreateClass(FENEWTONSTRATEGY_ID, "Broyden", febStep.GetFSModel()); break;
				default:
					assert(false);
				}
				qnProp->SetComponent(qnSolver);

				qnSolver->SetParamInt("max_ups", ops.ilimit);
			}
			else
			{
				copyParameter(log, solver, p);
			}
		}
	}
}

bool FSProject::ConvertDiscrete(std::ostream& log)
{
	FSModel& fem = GetFSModel();

	GModel& mdl = fem.GetModel();
	for (int i = 0; i < mdl.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = mdl.DiscreteObject(i);

		GDiscreteSpringSet* ps = dynamic_cast<GDiscreteSpringSet*>(po);
		if (ps)
		{
			FSDiscreteMaterial* pm = ps->GetMaterial();
			if (pm)
			{
				FSDiscreteMaterial* febMat = FEBio::CreateDiscreteMaterial(pm->GetTypeString(), &fem);

				if (dynamic_cast<FSNonLinearSpringMaterial*>(pm))
				{
					int m = pm->GetParam("measure")->GetIntValue();
					double s = pm->GetParam("scale")->GetFloatValue();

					Param* pF = pm->GetParam("force");
					double F = pF->GetFloatValue();

					febMat->SetParamInt("measure", m);
					febMat->SetParamFloat("scale", s*F);

					int lcid = pF->GetLoadCurveID();
					if (lcid >= 0)
					{
						FEBioLoadController* plc = dynamic_cast<FEBioLoadController*>(fem.GetLoadControllerFromID(lcid));
						LoadCurve& lc = *plc->CreateLoadCurve();

						FEBioFunction1D* pf = dynamic_cast<FEBioFunction1D*>(FEBio::CreateFunction1D("point", &fem));
						LoadCurve& f1d = *pf->CreateLoadCurve();
						f1d = lc;
						pf->UpdateData(true);

						FSProperty* pForce = febMat->FindProperty("force"); assert(pForce);
						if (pForce) pForce->SetComponent(pf);
					}
				}
				else
				{
					copyParameters(log, febMat, pm);
				}
				ps->SetMaterial(febMat);
				delete pm;
			}
		}
	}

	return true;
}
