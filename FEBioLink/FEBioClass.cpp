/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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
#include "FEBioClass.h"
#include "FEBioModule.h"
#include <FECore/FECoreKernel.h>
#include <FECore/FECoreBase.h>
#include <FECore/FEModelParam.h>
#include <FEBioLib/FEBioModel.h>
using namespace FEBio;

// dummy model used for allocating temporary FEBio classes.
static FEBioModel febioModel;

static std::map<std::string, int> classIndex;

int baseClassIndex(const char* sz)
{
	if (sz == nullptr) return -1;

	int n = -1;
	auto it = classIndex.find(sz);
	if (it == classIndex.end())
	{
		n = classIndex.size();
		classIndex[sz] = n;
	}
	else n = classIndex[sz];

	return n;
}

bool in_vector(const vector<int>& v, int n)
{
	for (int j = 0; j < v.size(); ++j)
	{
		if (v[j] == n) return true;
	}
	return false;
}

std::vector<FEBio::FEBioClassInfo> FEBio::FindAllClasses(int mod, int superId, int baseClassId, unsigned int flags)
{
	vector<FEBio::FEBioClassInfo> facs;

	bool includeModuleDependencies = (flags & ClassSearchFlags::IncludeModuleDependencies);
	bool includeFECoreClasses      = (flags & ClassSearchFlags::IncludeFECoreClasses);

	FECoreKernel& fecore = FECoreKernel::GetInstance();
	vector<int> mods;
	if ((mod != -1) && includeModuleDependencies)
	{
		mods = fecore.GetModuleDependencies(mod - 1);
	}
	if ((mod != -1) && includeFECoreClasses) 
	{
		mods.push_back(0);
	}

	for (int i = 0; i < fecore.FactoryClasses(); ++i)
	{
		const FECoreFactory* fac = fecore.GetFactoryClass(i);
		int facmod = fac->GetModuleID();

		int baseId = baseClassIndex(fac->GetBaseClassName());

		if (((superId     == -1) || (fac->GetSuperClassID() == superId)) && 
			((baseClassId == -1) || (baseId == baseClassId)))
		{
			if ((mod == -1) || (mod == facmod) || in_vector(mods, facmod))
			{
				const char* szmod = fecore.GetModuleName(fac->GetModuleID() - 1);
				FEBio::FEBioClassInfo febc = { fac->GetTypeStr(), szmod, (unsigned int) i };
				facs.push_back(febc);
			}
		}
	}

	return facs;
}

int FEBio::GetClassId(int superClassId, const std::string& typeStr)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return fecore.GetFactoryIndex(superClassId, typeStr.c_str());
}

FEBioParam& FEBioClass::AddParameter(const std::string& paramName, int paramType, const QVariant& val)
{
	FEBioParam p;
	p.m_type = paramType;
	p.m_name = paramName;
	p.m_val  = val;
	m_Param.push_back(p);
	return m_Param[m_Param.size() - 1];
}

void FEBioClass::AddProperty(const std::string& propName, int superClassId, int baseClassId)
{
	FEBioProperty prop;
	prop.m_name = propName;
	prop.m_baseClassId = baseClassId;
	prop.m_superClassId = superClassId;
	m_Props.push_back(prop);
}

QVariant vec3d_to_qvariant(const vec3d& v)
{
	QList<QVariant> val;
	val.push_back(v.x);
	val.push_back(v.y);
	val.push_back(v.z);
	return val;
}

QVariant mat3d_to_qvariant(const mat3d& m)
{
	QList<QVariant> val;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j) val.push_back(m[i][j]);
	return val;
}

QVariant mat3ds_to_qvariant(const mat3ds& m)
{
	QList<QVariant> val;
	val.push_back(m.xx());
	val.push_back(m.yy());
	val.push_back(m.zz());
	val.push_back(m.xy());
	val.push_back(m.yz());
	val.push_back(m.xz());
	return val;
}

FEBioClass* FEBio::CreateFEBioClass(int classId)
{
	// Get the kernel
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	// find the factory
	const FECoreFactory* fac = fecore.GetFactoryClass(classId); assert(fac);
	if (fac == nullptr) return nullptr;

	// try to create a temporary FEBio object
	unique_ptr<FECoreBase> pc(fac->Create(&febioModel)); assert(pc);
	if (pc == nullptr) return nullptr;

	const char* sztype = fac->GetTypeStr();

	// create the interface class
	FEBioClass* feb = new FEBioClass;
	feb->SetSuperClassID(fac->GetSuperClassID());
	feb->SetTypeString(sztype);

	// copy parameter info
	FEParameterList& pl = pc->GetParameterList();
	int params = pl.Parameters();
	FEParamIterator it = pl.first();
	for (int i = 0; i < params; ++i, ++it)
	{
		FEParam& p = *it;
		int ndim = p.dim();
		switch (p.type())
		{
		case FE_PARAM_INT   : 
		{
			FEBioParam& param = feb->AddParameter(p.name(), p.type(), p.value<int>());
			param.m_flags = p.GetFlags();
			if (p.enums())
			{
				param.m_enums = p.enums();
			}
		}
		break;
		case FE_PARAM_BOOL  : feb->AddParameter(p.name(), p.type(), p.value<bool>()); break;
		case FE_PARAM_DOUBLE: 
		{
			if (ndim == 1)
				feb->AddParameter(p.name(), p.type(), p.value<double>());
			else if (ndim == 3)
			{
				vec3d v(0, 0, 0);
				v.x = p.value<double>(0);
				v.y = p.value<double>(1);
				v.z = p.value<double>(2);
				feb->AddParameter(p.name(), FE_PARAM_VEC3D, vec3d_to_qvariant(v));
			}
			else assert(false);
		}
		break;
		case FE_PARAM_VEC3D : feb->AddParameter(p.name(), p.type(), vec3d_to_qvariant(p.value<vec3d>())); break;
		case FE_PARAM_MAT3D : feb->AddParameter(p.name(), p.type(), mat3d_to_qvariant(p.value<mat3d>())); break;
		case FE_PARAM_STD_STRING: feb->AddParameter(p.name(), p.type(), QString::fromStdString(p.value<std::string>())); break;
		case FE_PARAM_DOUBLE_MAPPED: 
		{
			if (ndim == 1)
			{
				FEParamDouble& v = p.value<FEParamDouble>();
				feb->AddParameter(p.name(), p.type(), v.constValue());
			}
			else if (ndim == 3)
			{
				vec3d v(0, 0, 0);
				v.x = p.value<FEParamDouble>(0).constValue();
				v.y = p.value<FEParamDouble>(1).constValue();
				v.z = p.value<FEParamDouble>(2).constValue();
				feb->AddParameter(p.name(), FE_PARAM_VEC3D, vec3d_to_qvariant(v));
			}
			else assert(false);
		}
		break;
		case FE_PARAM_VEC3D_MAPPED: 
		{
			vec3d v(0,0,0); // TODO: Grab const value from FEParamVec3d
			QVariant val = vec3d_to_qvariant(v);
			feb->AddParameter(p.name(), p.type(), val);
		}
		break;
		case FE_PARAM_MAT3D_MAPPED:
		{
			mat3d M; M.unit(); // TODO: Grab const value from FEParamMat3d
			QVariant val = mat3d_to_qvariant(M);
//			feb->AddParameter(p.name(), p.type(), val);
		}
		break;
		case FE_PARAM_MAT3DS_MAPPED:
		{
			mat3ds M; M.unit(); // TODO: Grab const value from FEParamMat3d
			QVariant val = mat3ds_to_qvariant(M);
			feb->AddParameter(p.name(), p.type(), val);
		}
		break;
		case FEBIO_PARAM_STD_VECTOR_INT:
		{
			// Don't know how to handle this.
		}
		break;
		case FEBIO_PARAM_STD_VECTOR_DOUBLE:
		{
			// Don't know how to handle this.
		}
		break;
		case FE_PARAM_DATA_ARRAY:
		{
			// Don't know how to handle this.
		}
		break;
		default:
			assert(false);
		}

		if (feb->Parameters() > 0)
		{
			FEBioParam& febParam = feb->GetParameter(feb->Parameters() - 1);
			if (p.units()) febParam.m_szunit = p.units();
		}
	}

	// copy properties
	int props = pc->PropertyClasses();
	for (int i = 0; i < props; ++i)
	{
		FEProperty* prop = pc->PropertyClass(i);
		const char* sz = prop->GetClassName();

		// lookup the base class ID.
		int n = baseClassIndex(sz);

		// add it
		feb->AddProperty(prop->GetName(), prop->GetSuperClassID(), n);
	}

	// all done!
	return feb;
}


vector<FEBio::FEBioModule>	FEBio::GetAllModules()
{
	// Get the kernel
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	vector<FEBio::FEBioModule> mods;
	for (int i = 0; i < fecore.Modules(); ++i)
	{
		FEBio::FEBioModule mod;
		mod.m_szname = fecore.GetModuleName(i);
		mod.m_szdesc = fecore.GetModuleDescription(i);
		mod.m_id = i + 1;
		mods.push_back(mod);
	}

	return mods;
}

const char* FEBio::GetModuleName(int moduleId)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	const char* szmodName = fecore.GetModuleNameFromId(moduleId); assert(szmodName);
	return szmodName;
}

int FEBio::GetModuleId(const std::string& moduleName)
{
	vector<FEBio::FEBioModule> modules = GetAllModules();
	for (int i = 0; i < modules.size(); ++i)
	{
		FEBioModule& mod = modules[i];
		string modName(mod.m_szname);
		if (modName == moduleName) return mod.m_id;
	}
	assert(false);
	return -1;
}

void FEBio::SetActiveModule(int moduleID)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	fecore.SetActiveModule(moduleID);
}

int FEBio::GetActiveModule()
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return fecore.GetActiveModule();
}