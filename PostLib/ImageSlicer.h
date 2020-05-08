#pragma once
#include <ImageLib/3DImage.h>
#include <FSCore/box.h>
#include "GLImageRenderer.h"
#include <ImageLib/RGBAImage.h>
#include "ColorMap.h"

namespace Post {

class CImageModel;

class CImageSlicer : public CGLImageRenderer
{
	enum { ORIENTATION, OFFSET, COLOR_MAP };

public:
	CImageSlicer(CImageModel* img);
	~CImageSlicer();

	void Create();

	void Update() override;

	void Render(CGLContext& rc) override;

	int GetOrientation() const { return m_op; }
	void SetOrientation(int n) { m_op = n; }

	double GetOffset() const { return m_off; }
	void SetOffset(double f) { m_off = f; }

	int GetColorMap() const { return m_Col.GetColorMap(); }
	void SetColorMap(int n) { m_Col.SetColorMap(n); }

	bool UpdateData(bool bsave = true) override;

private:
	void BuildLUT();

	void UpdateSlice();

private:
	C3DImage		m_im3d;	// resampled 3D image data
	CRGBAImage		m_im;	// 2D image that will be displayed
	int				m_LUTC[4][256];	// color lookup table
	bool			m_reloadTexture;

	int		m_op;	// x,y,z
	double	m_off;	// offset (0 - 1)

	Post::CColorTexture	m_Col;

	unsigned int m_texID;
};
}
