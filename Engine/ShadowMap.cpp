#include "pch.h"
#include "ShadowMap.h"

ShadowMap::ShadowMap(float width, float height)
{
	CreateShaderMapTexture(width, height);
	CreateDepthStencilView();
	CreateShaderResourceView();
}

ShadowMap::~ShadowMap()
{
	
}

void ShadowMap::ClearDepthStencilView()
{
	DC->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMap::CreateShaderMapTexture(float width, float height)
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

void ShadowMap::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HRESULT hr = DEVICE->CreateDepthStencilView(_shaderMapTexture.Get(), &desc, _dsv.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void ShadowMap::CreateShaderResourceView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;

	DEVICE->CreateShaderResourceView(_shaderMapTexture.Get(), &desc, _srv.GetAddressOf());
}
