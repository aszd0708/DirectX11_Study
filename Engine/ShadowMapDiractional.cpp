#include "pch.h"
#include "ShadowMapDiractional.h"
#include "ShadowMapBase.h"
#include "Light.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"
#include "Camera.h"

ShadowMapDiractional::ShadowMapDiractional(float width, float height) : ShadowMapBase()
{
	Create(width, height);
}

ShadowMapDiractional::~ShadowMapDiractional()
{
	
}

void ShadowMapDiractional::ClearDepthStencilView(int index)
{
	if (_dsvs[index])
	{
		DC->ClearDepthStencilView(_dsvs[index].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

void ShadowMapDiractional::Create(uint32 width, uint32 height)
{
	_width = width;
	_height = height;

	CreateShaderMapTexture(width, height);
	CreateDepthStencilView();
	CreateShaderResourceView();
}

void ShadowMapDiractional::BindRTVAndDSV()
{
	Viewport vp(_width, _height);
	vp.RSSetViewport();
}

Matrix ShadowMapDiractional::GetLightView(const eShadowMapType& shadowMapType, shared_ptr<Light> light)
{
	LightDesc desc = light->GetLightDesc();
	Vec3 dir = desc.direction; // 고정값 또는 ImGui 슬라이더 (카메라 Look 금지)

	Matrix invView = Camera::S_MatView.Invert();
	Vec3 camPos = invView.Translation();
	Vec3 camForward = Vec3(invView._31, invView._32, invView._33);
	camForward.Normalize();

	float centerDist = 0.0f;
	switch (shadowMapType)
	{
	case eShadowMapType::Near: centerDist = 7.5f;   break;
	case eShadowMapType::Mid:  centerDist = 37.5f;  break;
	case eShadowMapType::Far:  centerDist = 180.0f; break;
	}
	Vec3 center = camPos + camForward * centerDist;

	Vec3 eye = center - dir * 300.0f;
	return ::XMMatrixLookAtLH(eye, center, Vec3(0, 1, 0));
}

Matrix ShadowMapDiractional::GetLightProj(const eShadowMapType& shadowMapType, shared_ptr<Light> light)
{
	float shadowWidth = 0.0f;
	switch (shadowMapType)
	{
		case eShadowMapType::Near:
		{
			shadowWidth = 30.0f;
			break;
		}
	
		case eShadowMapType::Mid:
		{
			shadowWidth = 120;
			break;
		}
	
		case eShadowMapType::Far:
		{
			shadowWidth = 500;
			break;
		}
	}
	
	float shadowHeight = shadowWidth;
	float shadowNear = 0.1f;
	float shadowFar = 1000.0f;
	return ::XMMatrixOrthographicLH(shadowWidth, shadowHeight, shadowNear, shadowFar);

	float angle = light->GetLightDesc().angle;
	float range = light->GetLightDesc().range;
	float fovY = ::XMConvertToRadians(angle);
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 0.1f, range);
}

Matrix ShadowMapDiractional::GetLightVP(const eShadowMapType& shadowMapType, shared_ptr<Light> light)
{
	return GetLightView(shadowMapType, light) * GetLightProj(shadowMapType, light);
}

void ShadowMapDiractional::RenderShadowMap(shared_ptr<Light> light, shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objects)
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();

	for (int shadowMapType = (int)ShadowMapDiractional::eShadowMapType::Near; shadowMapType < ShadowMapDiractional::eShadowMapType::MAX; ++shadowMapType)
	{
		ShadowMapDiractional::eShadowMapType type = (ShadowMapDiractional::eShadowMapType)shadowMapType;
		ClearDepthStencilView(shadowMapType);

		DC->OMSetRenderTargets(0, nullptr, GetDSV(shadowMapType).Get());

		shader->PushGlobalData(GetLightView(type, light), GetLightProj(type, light));

		SetShadowPass(shader, objects);
	}

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

ShadowMapBase::ShadowDesc ShadowMapDiractional::CreateShadowBuffer(shared_ptr<Light> light)
{
	ShadowDesc shadowBuffer;
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Near] = GetLightVP(ShadowMapDiractional::eShadowMapType::Near, light);
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Mid] = GetLightVP(ShadowMapDiractional::eShadowMapType::Mid, light);
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Far] = GetLightVP(ShadowMapDiractional::eShadowMapType::Far, light);
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	return shadowBuffer;
}

void ShadowMapDiractional::CreateShaderMapTexture(float width, float height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = (int)eShadowMapType::MAX;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	DEVICE->CreateTexture2D(&desc, nullptr, _shaderMapTexture.GetAddressOf());
}

void ShadowMapDiractional::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;

	for (int i = 0; i < eShadowMapType::MAX; ++i)
	{
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.FirstArraySlice = i;
		desc.Texture2DArray.ArraySize = 1;

		HRESULT hr = DEVICE->CreateDepthStencilView(_shaderMapTexture.Get(), &desc, _dsvs[i].GetAddressOf());
		assert(SUCCEEDED(hr));
	}
}

void ShadowMapDiractional::CreateShaderResourceView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MostDetailedMip = 0;
	desc.Texture2DArray.MipLevels = 1;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = (int)eShadowMapType::MAX;

	DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &desc, _srv.GetAddressOf());

	// ImGUI 디버깅 용
	D3D11_SHADER_RESOURCE_VIEW_DESC layerSrvDesc = {};
	layerSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	layerSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	layerSrvDesc.Texture2DArray.MostDetailedMip = 0;
	layerSrvDesc.Texture2DArray.MipLevels = 1;
	layerSrvDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < eShadowMapType::MAX; ++i)
	{
		layerSrvDesc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &layerSrvDesc, _srvs[i].GetAddressOf());
		CHECK(hr);
	}
}
