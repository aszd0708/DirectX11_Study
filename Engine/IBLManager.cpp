#include "pch.h"
#include "IBLManager.h"
#include "Utils.h"

void IBLManager::Bake(shared_ptr<Texture> hdrMap, shared_ptr<Texture>& outIrradiance, shared_ptr<Texture>& outPrefiltered, shared_ptr<Texture>& outBRDF)
{
	shared_ptr<Texture> cubeMap = CreateEnvironmentCubemap(hdrMap);
	outIrradiance = CreateIrradiance(cubeMap);
	outPrefiltered = CreatePrefiltered(cubeMap);
	outBRDF = CreateBRDF();
}

shared_ptr<Texture> IBLManager::CreateEnvironmentCubemap(shared_ptr<Texture> hdrMap)
{
	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->CreateCubeMap(512, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);

	ComPtr<ID3D11UnorderedAccessView> uav;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 6;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.MipSlice = 0;
	HRESULT hr = DEVICE->CreateUnorderedAccessView(texture->GetTexture2D().Get(), &uavDesc, uav.GetAddressOf());
	CHECK(hr);

	shared_ptr<Shader> shader = make_shared<Shader>(L"EquiToCube.fx");
	shader->GetSRV("InputMap")->SetResource(hdrMap->GetSRV().Get());
	shader->GetUAV("OutputMap")->SetUnorderedAccessView(uav.Get());

	UINT groupX = 512 / 8;
	UINT groupY = 512 / 8;
	UINT groupZ = 6;
	shader->Dispatch(0, 0, groupX, groupY, groupZ);
	DC->Flush();

	shader->GetUAV("OutputMap")->SetUnorderedAccessView(nullptr);
	return texture;
}

shared_ptr<Texture> IBLManager::CreateIrradiance(shared_ptr<Texture> hdrMap)
{
	ComPtr<ID3D11UnorderedAccessView> uav;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = 32;
	texDesc.Height = 32;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	ComPtr<ID3D11Texture2D> texture2D;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, nullptr, texture2D.GetAddressOf());
	CHECK(hr);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = 6;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.MipSlice = 0;
	hr = DEVICE->CreateUnorderedAccessView(texture2D.Get(), &uavDesc, uav.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> srv;
	hr = DEVICE->CreateShaderResourceView(texture2D.Get(), &srvDesc, srv.GetAddressOf());
	CHECK(hr);

	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->SetTexture(texture2D, srv);

	shared_ptr<Shader> shader = make_shared<Shader>(L"Irradiance.fx");
	shader->GetSRV("InputMap")->SetResource(hdrMap->GetSRV().Get());
	shader->GetUAV("OutputMap")->SetUnorderedAccessView(uav.Get());

	UINT groupX = 32 / 8; 
	UINT groupY = 32 / 8; 
	UINT groupZ = 6;      
	shader->Dispatch(0, 0, groupX, groupY, groupZ);
	DC->Flush();

	shader->GetUAV("OutputMap")->SetUnorderedAccessView(nullptr);
	return texture;
}

shared_ptr<Texture> IBLManager::CreatePrefiltered(shared_ptr<Texture> hdrMap)
{
	vector<ComPtr<ID3D11UnorderedAccessView>> uavs;
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = 512;
	texDesc.Height = 512;
	texDesc.MipLevels = 5;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	ComPtr<ID3D11Texture2D> texture2D;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, nullptr, texture2D.GetAddressOf());
	CHECK(hr);

	for (int i = 0; i < 5; ++i)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.ArraySize = 6;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.MipSlice = i;
		ComPtr<ID3D11UnorderedAccessView> uav;
		hr = DEVICE->CreateUnorderedAccessView(texture2D.Get(), &uavDesc, uav.GetAddressOf());
		uavs.emplace_back(uav);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 5;
	srvDesc.TextureCube.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> srv;
	hr = DEVICE->CreateShaderResourceView(texture2D.Get(), &srvDesc, srv.GetAddressOf());
	CHECK(hr);

	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->SetTexture(texture2D, srv);


	shared_ptr<Shader> shader = make_shared<Shader>(L"Prefiltered.fx");
	shared_ptr<ConstantBuffer<sPrefilteredDesc>> buffer = make_shared<ConstantBuffer<sPrefilteredDesc>>();
	buffer->Create();
	for (int mipLevel = 0; mipLevel < 5; ++mipLevel)
	{
		sPrefilteredDesc desc;
		desc.mipLevel = mipLevel;
		desc.roughness = (float)mipLevel / (5 - 1);
		desc.resolution = 512 >> mipLevel;
		buffer->CopyData(desc);

		shader->GetConstantBuffer("PrefilteredBuffer")->SetConstantBuffer(buffer->GetComPtr().Get());
		shader->GetSRV("InputMap")->SetResource(hdrMap->GetSRV().Get());
		shader->GetUAV("OutputMap")->SetUnorderedAccessView(uavs[mipLevel].Get());

		UINT groupX = desc.resolution / 8;
		UINT groupY = desc.resolution / 8;
		UINT groupZ = 6;
		shader->Dispatch(0, 0, groupX, groupY, groupZ);
		DC->Flush();
	}

	shader->GetUAV("OutputMap")->SetUnorderedAccessView(nullptr);
	return texture;
}

shared_ptr<Texture> IBLManager::CreateBRDF()
{
	ComPtr<ID3D11UnorderedAccessView> uav;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = 512;
	texDesc.Height = 512;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.MiscFlags = 0;
	ComPtr<ID3D11Texture2D> texture2D;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, nullptr, texture2D.GetAddressOf());
	CHECK(hr);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	hr = DEVICE->CreateUnorderedAccessView(texture2D.Get(), &uavDesc, uav.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> srv;
	hr = DEVICE->CreateShaderResourceView(texture2D.Get(), &srvDesc, srv.GetAddressOf());
	CHECK(hr);

	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->SetTexture(texture2D, srv);

	shared_ptr<Shader> shader = make_shared<Shader>(L"BRDF.fx");
	shader->GetUAV("OutputMap")->SetUnorderedAccessView(uav.Get());

	UINT groupX = 512 / 8;
	UINT groupY = 512 / 8;
	UINT groupZ = 1;
	shader->Dispatch(0, 0, groupX, groupY, groupZ);
	DC->Flush();

	shader->GetUAV("OutputMap")->SetUnorderedAccessView(nullptr);

	return texture;
}

