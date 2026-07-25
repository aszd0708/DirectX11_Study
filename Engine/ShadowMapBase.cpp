#include "pch.h"
#include "ShadowMapBase.h"

ShadowMapBase::ShadowMapBase()
{
}

ShadowMapBase::~ShadowMapBase()
{
}

void ShadowMapBase::Create(uint32 width, uint32 height)
{
	_width = width;
	_height = height;
}

void ShadowMapBase::BindRTVAndDSV()
{

	Viewport vp(_width, _height);
	vp.RSSetViewport();
}
