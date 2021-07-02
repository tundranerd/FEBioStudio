#pragma once
#include "FEBase.h"
#include "FEDataMap.h"

//-----------------------------------------------------------------------------
// Base class for components of an FEModel
class FEModelComponent : public FEBase
{
public:
	FEModelComponent();

	int GetSuperClassID() const;

	// Set the super class ID. This should not be called directly,
	// but now some material classes need to set their super class to FEMATERIALPROP_ID
	void SetSuperClassID(int superClassID);

	virtual void SetTypeString(const char* sztype);
	virtual const char* GetTypeString();

protected:
	int			m_superClassID;		// super class ID (defined in enums.h)
	const char* m_sztype;			// type string
};


void SaveClassMetaData(FEModelComponent* pc, OArchive& ar);
void LoadClassMetaData(FEModelComponent* pc, IArchive& ar);
