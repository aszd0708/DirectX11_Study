#include "pch.h"
#include "Shadow.h"
#include "ShadowMapBase.h"

Shadow::Shadow(LightDesc::eLightType type, uint32 width, uint32 height)
{
	_type = type;
	_shader = make_shared<Shader>(L"ShadowMap.fx");

	switch (type)
	{
		case LightDesc::eLightType::Directional:
			_shadowMap = make_shared<ShadowMapDiractional>(width, height);
		break;
		case LightDesc::eLightType::Spot:
			_shadowMap = make_shared<ShadowMapSpot>(width, height);
		break;
		case LightDesc::eLightType::Point:
			_shadowMap = make_shared<ShadowMapPoint>(width, height);
		break;
	}
	_shadowBuffer= make_shared<ConstantBuffer<ShadowMapBase::ShadowDesc>>();
	_shadowBuffer->Create();
}

Shadow::~Shadow()
{

}

void Shadow::Render(shared_ptr<Light> light, vector<shared_ptr<GameObject>>& objects)
{
	_shadowMap->RenderShadowMap(light, _shader, objects);
}

void Shadow::CreateShadowBuffer(shared_ptr<Light> light)
{
	ShadowMapBase::ShadowDesc desc = _shadowMap->CreateShadowBuffer(light);
	_shadowBuffer->CopyData(desc);
}

void Shadow::ApplyShadow(shared_ptr<Shader> shader, LightDesc::eLightType type)
{
	string shadowResourceKey;
	switch (type)
	{
	case (int)LightDesc::eLightType::Directional:
		shadowResourceKey = "ShadowMapArray";
		break;
	case (int)LightDesc::eLightType::Spot:
		shadowResourceKey = "ShadowMapSpot";
		break;
	case (int)LightDesc::eLightType::Point:
		shadowResourceKey = "ShadowMapCubePoint";
		break;
	}

	shader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(GetShadowBuffer()->GetComPtr().Get());
	shader->GetSRV(shadowResourceKey)->SetResource(GetShadowMap()->GetSRV().Get());
}

ComPtr<ID3D11ShaderResourceView> Shadow::GetLayerSRV(int index)
{
	return _shadowMap->GetLayerSRV(index);
}
