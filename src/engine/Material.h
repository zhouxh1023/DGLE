/**
\author		Korotkov Andrey aka DRON
\date		15.04.2013 (c)Korotkov Andrey

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#pragma once

#include "Common.h"
#include "ResourceManager.h"

class CMaterial: public CInstancedObj, public IMaterial
{
	TColor4 _stDiffCol, _stSpecCol;
	float _fShine;
	ITexture *_pDiffTex;
	
	bool _doBlending;
	E_BLENDING_EFFECT _eBlendingMode;
	
	bool _doAlphaTest;
	float _fAlphaTreshold;

public:

	CMaterial(uint uiInstIdx);

	DGLE_RESULT DGLE_API SetDiffuseColor(const TColor4 &stColor);
	DGLE_RESULT DGLE_API SetSpecularColor(const TColor4 &stColor);
	DGLE_RESULT DGLE_API SetShininess(float fShininess);
	DGLE_RESULT DGLE_API SetDiffuseTexture(ITexture *pTexture);
	DGLE_RESULT DGLE_API SetBlending(bool bEnabled, E_BLENDING_EFFECT eMode);
	DGLE_RESULT DGLE_API SetAlphaTest(bool bEnabled, float fTreshold);

	DGLE_RESULT DGLE_API GetDiffuseColor(TColor4 &stColor);
	DGLE_RESULT DGLE_API GetSpecularColor(TColor4 &stColor);
	DGLE_RESULT DGLE_API GetShininess(float &fShininess);
	DGLE_RESULT DGLE_API GetDiffuseTexture(ITexture *&prTexture);
	DGLE_RESULT DGLE_API GetBlending(bool &bEnabled, E_BLENDING_EFFECT &eMode);
	DGLE_RESULT DGLE_API GetAlphaTest(bool &bEnabled, float &fTreshold);

	DGLE_RESULT DGLE_API Bind();

	IENGINE_BASE_OBJECT_IMPLEMENTATION(EOT_MATERIAL)
	IDGLE_BASE_IMPLEMENTATION(IMaterial, INTERFACE_IMPL(IEngineBaseObject, INTERFACE_IMPL_END))
};