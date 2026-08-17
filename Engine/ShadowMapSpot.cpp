#include "pch.h"
#include "ShadowMapSpot.h"
#include "ShadowMapBase.h"
#include "Light.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"

ShadowMapSpot::ShadowMapSpot(uint32 width, uint32 height) : ShadowMapBase()
{
	Create(width, height);
}

ShadowMapSpot::~ShadowMapSpot()
{

}

void ShadowMapSpot::ClearDepthStencilView()
{
	DC->ClearDepthStencilView(_dsvs.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMapSpot::Create(uint32 width, uint32 height)
{
	ShadowMapBase::Create(width, height);

	CreateShaderMapTexture(width, height);
	CreateDepthStencilView();
	CreateShaderResourceView();
}

void ShadowMapSpot::BindRTVAndDSV()
{
	ShadowMapBase::BindRTVAndDSV();
}

Matrix ShadowMapSpot::GetLightView(shared_ptr<Light> light)
{
	LightDesc desc = light->GetLightDesc();
	Vec3 dir = desc.direction;
	dir.Normalize();
	return ::XMMatrixLookAtLH(desc.position, desc.position + dir, Vec3(0, 1, 0));
}

Matrix ShadowMapSpot::GetLightProj(shared_ptr<Light> light)
{
	LightDesc desc = light->GetLightDesc();
	float angle = desc.angle * 2;
	float range = desc.range;
	float fovY = ::XMConvertToRadians(XMMin(XMMax(angle, 1.0f), 170.0f));
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 1.0f, range);
}

Matrix ShadowMapSpot::GetLightVP(shared_ptr<Light> light)
{
	return GetLightView(light) * GetLightProj(light);
}

void ShadowMapSpot::RenderShadowMap(shared_ptr<Light> light, shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objects)
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();

	ClearDepthStencilView();

	DC->OMSetRenderTargets(0, nullptr, GetDSV().Get());

	shader->PushGlobalData(GetLightView(light), GetLightProj(light));

	SetShadowPass(shader, objects);

	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* nullDSV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

ShadowMapBase::ShadowDesc ShadowMapSpot::CreateShadowBuffer(shared_ptr<Light> light)
{
	Matrix vp = GetLightVP(light);
	ShadowDesc shadowBuffer;
	shadowBuffer.lightVP[0] = vp;
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	return shadowBuffer;
}

void ShadowMapSpot::CreateShaderMapTexture(float width, float height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	DEVICE->CreateTexture2D(&desc, nullptr, _shaderMapTexture.GetAddressOf());
}

void ShadowMapSpot::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	
	DEVICE->CreateDepthStencilView(_shaderMapTexture.Get(), &desc, _dsvs.GetAddressOf());
}

void ShadowMapSpot::CreateShaderResourceView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;
	
	DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &desc, _srv.GetAddressOf());
}
