#include "pch.h"
#include "ShadowMapPoint.h"
#include "ShadowMapBase.h"
#include "Light.h"

ShadowMapPoint::ShadowMapPoint(uint32 width, uint32 height) : ShadowMapBase()
{
	Create(width, height);
}

ShadowMapPoint::~ShadowMapPoint()
{

}

void ShadowMapPoint::ClearDepthStencilView(int index)
{
	if (_dsvs[index])
	{
		DC->ClearDepthStencilView(_dsvs[index].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

void ShadowMapPoint::Create(uint32 width, uint32 height)
{
	ShadowMapBase::Create(width, height);

	CreateShaderMapTexture(width, height);
	CreateDepthStencilView();
	CreateShaderResourceView();
}

void ShadowMapPoint::BindRTVAndDSV()
{
	Viewport vp(_width, _height);
	vp.RSSetViewport();
}

Matrix ShadowMapPoint::GetLightView(shared_ptr<Light> light, const int& index)
{
	LightDesc desc = light->GetLightDesc();
	Vec3 lookVector = ShadowMapPoint::LOOK_VECTOR[index];
	Vec3 upVector = ShadowMapPoint::UP_VECTOR[index];
	return ::XMMatrixLookAtLH(desc.position, desc.position + lookVector, upVector);
}

Matrix ShadowMapPoint::GetLightProj(shared_ptr<Light> light)
{
	float range = light->GetLightDesc().range;
	float fovY = DirectX::XM_PIDIV2;
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 1.0f, range);
}

Matrix ShadowMapPoint::GetLightVP(shared_ptr<Light> light, int& index)
{
	return GetLightView(light, index) * GetLightProj(light);
}

void ShadowMapPoint::RenderShadowMap(shared_ptr<Light> light, shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objects)
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();

	for (int i = 0; i < ShadowMapPoint::MAX_TEXTURE_COUNT; ++i)
	{
		ClearDepthStencilView(i);

		DC->OMSetRenderTargets(0, nullptr, GetDSV(i).Get());

		shader->PushGlobalData(GetLightView(light, i), GetLightProj(light));

		SetShadowPass(shader, objects);
	}

	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* nullDSV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

ShadowMapBase::ShadowDesc ShadowMapPoint::CreateShadowBuffer(shared_ptr<Light> light)
{
	ShadowDesc shadowBuffer;
	Matrix lightProj = GetLightProj(light);
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	shadowBuffer.lightProjValues = Vec2(lightProj._33, lightProj._43);
	return shadowBuffer;
}

void ShadowMapPoint::CreateShaderMapTexture(float width, float height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = MAX_TEXTURE_COUNT;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = DEVICE->CreateTexture2D(&desc, nullptr, _shaderMapTexture.GetAddressOf());
	CHECK(hr);
}

void ShadowMapPoint::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	
	for (int i = 0; i < MAX_TEXTURE_COUNT; ++i)
	{
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.FirstArraySlice = i;
		desc.Texture2DArray.ArraySize = 1;

		HRESULT hr = DEVICE->CreateDepthStencilView(_shaderMapTexture.Get(), &desc, _dsvs[i].GetAddressOf());
		CHECK(hr);
	}
}

void ShadowMapPoint::CreateShaderResourceView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	desc.Texture2DArray.ArraySize = MAX_TEXTURE_COUNT;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.MipLevels = 1;
	desc.Texture2DArray.MostDetailedMip = 0;

	HRESULT hr2 = DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &desc, _srv.GetAddressOf());
	CHECK(hr2);

	// ImGUI 디버깅 용
	D3D11_SHADER_RESOURCE_VIEW_DESC layerSrvDesc = {};
	layerSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	layerSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	layerSrvDesc.Texture2DArray.MostDetailedMip = 0;
	layerSrvDesc.Texture2DArray.MipLevels = 1;
	layerSrvDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < MAX_TEXTURE_COUNT; ++i)
	{
		layerSrvDesc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &layerSrvDesc, _srvs[i].GetAddressOf());
		CHECK(hr);
	}
}
