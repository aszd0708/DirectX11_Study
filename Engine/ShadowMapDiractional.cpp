#include "pch.h"
#include "ShadowMapDiractional.h"
#include "ShadowMapBase.h"

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
