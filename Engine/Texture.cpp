#include "pch.h"
#include "Texture.h"

Texture::Texture() : Super(ResourceType::Texture)
{

}

Texture::~Texture()
{

}

void Texture::Load(const wstring& path)
{
	DirectX::TexMetadata md;
	HRESULT hr = ::LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &md, _img);
	CHECK(hr);

	if (_isSRGB)
	{
		md.format = DirectX::MakeSRGB(md.format);
		_img.OverrideFormat(md.format);
	}

	hr = ::CreateShaderResourceView(DEVICE.Get(), _img.GetImages(), _img.GetImageCount(), md, _shaderResourveView.GetAddressOf());
	CHECK(hr);
	
	_size.x = md.width;
	_size.y = md.height;
}

void Texture::LoadHDR(const wstring& path)
{
	DirectX::TexMetadata md;

	HRESULT hr = DirectX::LoadFromHDRFile(path.c_str(), &md, _img);
	CHECK(hr);

	hr = ::CreateShaderResourceView(DEVICE.Get(), _img.GetImages(),
		_img.GetImageCount(),
		md, _shaderResourveView.GetAddressOf());
	CHECK(hr);

	_size.x = md.width;
	_size.y = md.height;
}

void Texture::SetTexture(ComPtr<ID3D11Texture2D> tex2D, ComPtr<ID3D11ShaderResourceView> srv)
{
	_shaderResourveView = srv;
	if (tex2D)
	{
		D3D11_TEXTURE2D_DESC desc;
		tex2D->GetDesc(&desc);
		_size.x = (float)desc.Width;
		_size.y = (float)desc.Height;
	}
}

ComPtr<ID3D11Texture2D> Texture::GetTexture2D()
{
	ComPtr<ID3D11Texture2D> texture;
	_shaderResourveView->GetResource((ID3D11Resource**)texture.GetAddressOf());
	return texture;
}

void Texture::CreateCubeMap(UINT resolution, DXGI_FORMAT format, UINT mipLevels)
{
	ComPtr<ID3D11Texture2D> texture2D;
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.ArraySize = 6;
		desc.Format = format;
		desc.Width = resolution;
		desc.Height = resolution;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.SampleDesc.Count = 1;
		desc.MipLevels = mipLevels;

		HRESULT hr = DEVICE->CreateTexture2D(&desc, nullptr, texture2D.GetAddressOf());
		CHECK(hr);
	}
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = mipLevels;
		desc.TextureCube.MostDetailedMip = 0;

		HRESULT hr = DEVICE->CreateShaderResourceView(texture2D.Get(), &desc, _shaderResourveView.GetAddressOf());
		CHECK(hr);
	}
	
	{
		_size.x = resolution;
		_size.y = resolution;
	}
}