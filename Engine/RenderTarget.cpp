#include "pch.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget(float width, float height)
{
	CreateRenderTarget(width, height);
	CreateRenderTargetView();
	CreateShaderResourceView();
}

RenderTarget::~RenderTarget()
{

}

void RenderTarget::ClearRenderTargetView()
{
	Color clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
	DC->ClearRenderTargetView(_renderTargetView.Get(), (float*)(&clearColor));
}

void RenderTarget::CreateRenderTarget(float width, float height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;


	DEVICE->CreateTexture2D(&desc, nullptr, _renderTarget.GetAddressOf());
}

void RenderTarget::CreateRenderTargetView()
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	DEVICE->CreateRenderTargetView(_renderTarget.Get(), &desc, _renderTargetView.GetAddressOf());
}

void RenderTarget::CreateShaderResourceView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;

	DEVICE->CreateShaderResourceView(_renderTarget.Get(), &desc, _shaderResourceView.GetAddressOf());
}
