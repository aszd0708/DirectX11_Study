#include "pch.h"
#include "AOBase.h"
#include "GeometryHelper.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"
#include "Camera.h"
#include "RenderTarget.h"

AOBase::AOBase(int width, int height, wstring shaderPath) : _width(width), _height(height)
{
	_shader = make_shared<Shader>(shaderPath);
	Init();
}

AOBase::~AOBase()
{

}

void AOBase::RenderAO(vector<shared_ptr<GameObject>> objs)
{
	_depthRenderTarget->ClearRenderTargetView();
	DC->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	Viewport vp((float)_width, (float)_height);
	vp.RSSetViewport();

	DC->OMSetRenderTargets(1, _depthRenderTarget->GetRenderTargetView().GetAddressOf(), _depthStencilView.Get());

	_shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	for (shared_ptr<GameObject>& obj : objs)
	{
		shared_ptr<ModelRenderer> modelRenderer = obj->GetModelRenderer();
		if (modelRenderer != nullptr)
		{
			modelRenderer->SetPass((int)eAOPass::NormalDepth);
			modelRenderer->Render(_shader);
		}
		shared_ptr<ModelAnimator> modelAnimator = obj->GetModelAnimator();
		if (modelAnimator != nullptr)
		{
			modelAnimator->SetPass((int)eAOPass::NormalDepth);
			modelAnimator->Render(_shader);
		}
		shared_ptr<MeshRenderer> meshRenderer = obj->GetMeshRenderer();
		if (meshRenderer != nullptr)
		{
			meshRenderer->SetPass((int)eAOPass::NormalDepth);
			meshRenderer->Render(_shader);
		}
	}

	CreateQuard();

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void AOBase::RenderBilateralBlur()
{
	_rawAORenderTarget->ClearRenderTargetView();
	DC->OMSetRenderTargets(1, _rawAORenderTarget->GetRenderTargetView().GetAddressOf(), nullptr);

	_shader->GetSRV("NormalDepthMap")->SetResource(_depthRenderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("NoiseTexture")->SetResource(_noiseSRV.Get());

	CreateQuard();

	uint32 stride = _quadVertexBuffer->GetStride();
	uint32 offset = _quadVertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _quadVertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_quadIndexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, (int)eAOPass::AO, _quadIndexBuffer->GetCount());

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void AOBase::ApplyAO(shared_ptr<Shader> shader)
{
	shader->GetSRV("PostProccessAOMap")->SetResource(_aoRenderTarget->GetShaderResourceView().Get());
	shader->GetConstantBuffer("AOSizeBuffer")->SetConstantBuffer(_screenSizeBuffer->GetComPtr().Get());
}

void AOBase::Render()
{
	_aoRenderTarget->ClearRenderTargetView();
	DC->OMSetRenderTargets(1, _aoRenderTarget->GetRenderTargetView().GetAddressOf(), nullptr);

	_shader->GetSRV("AOMap")->SetResource(_rawAORenderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("NormalDepthMap")->SetResource(_depthRenderTarget->GetShaderResourceView().Get());

	CreateQuard();

	uint32 stride = _quadVertexBuffer->GetStride();
	uint32 offset = _quadVertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _quadVertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_quadIndexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, (int)eAOPass::BilateralBlur, _quadIndexBuffer->GetCount());

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void AOBase::OnResize(int width, int height)
{
	_width = width;
	_height = height;

	if(_depthStencilView == nullptr || _depthStencilTexture == nullptr || _depthRenderTarget == nullptr || _aoRenderTarget == nullptr || _rawAORenderTarget == nullptr)
		return;

	_depthStencilView.Reset();
	_depthStencilTexture.Reset();

	ScreenSizeDesc screenDesc;
	screenDesc.width = width;
	screenDesc.height = height;
	_screenSizeBuffer->CopyData(screenDesc);
	_shader->GetConstantBuffer("AOSizeBuffer")->SetConstantBuffer(_screenSizeBuffer->GetComPtr().Get());

	_depthRenderTarget = make_shared<RenderTarget>(_width, _height);
	_aoRenderTarget = make_shared<RenderTarget>(_width, _height);
	_rawAORenderTarget = make_shared<RenderTarget>(_width, _height);

	CreaateDepthStencilView();
}

void AOBase::Init()
{
	ScreenSizeDesc screenDesc;
	screenDesc.width = GAME->GetGameDesc().width;
	screenDesc.height = GAME->GetGameDesc().height;
	_screenSizeBuffer = make_shared< ConstantBuffer<ScreenSizeDesc>>();
	_screenSizeBuffer->Create();
	_screenSizeBuffer->CopyData(screenDesc);
	_shader->GetConstantBuffer("AOSizeBuffer")->SetConstantBuffer(_screenSizeBuffer->GetComPtr().Get());

	_depthRenderTarget = make_shared<RenderTarget>(_width, _height);
	_aoRenderTarget = make_shared<RenderTarget>(_width, _height);
	_rawAORenderTarget = make_shared<RenderTarget>(_width, _height);

	shared_ptr<Geometry<VertexTextureNormalTangentData>> geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateQuad(geometry);
	_quadIndexBuffer = make_shared<IndexBuffer>();
	_quadIndexBuffer->Create(geometry->GetIndices());
	_quadVertexBuffer = make_shared<VertexBuffer>();
	_quadVertexBuffer->Create(geometry->GetVertices());

	CreateRandomNoiseTexture();
	CreaateDepthStencilView();
}

void AOBase::CreaateDepthStencilView()
{
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = static_cast<uint32>(GAME->GetGameDesc().width);
		desc.Height = static_cast<uint32>(GAME->GetGameDesc().height);
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = DEVICE->CreateTexture2D(&desc, nullptr, _depthStencilTexture.GetAddressOf());
		CHECK(hr);
	}

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		HRESULT hr = DEVICE->CreateDepthStencilView(_depthStencilTexture.Get(), &desc, _depthStencilView.GetAddressOf());
		CHECK(hr);
	}
}

void AOBase::CreateQuard()
{
	TransformDesc transformDesc;
	transformDesc.W = Matrix::CreateScale(2.0f);
	_shader->PushTransformData(transformDesc);
	_shader->PushGlobalData(Matrix::Identity, Matrix::Identity);
}

void AOBase::CreateRandomNoiseTexture()
{
	const int width = 4;
	const int height = 4;
	const int size = width * height;
	const double PI = 3.1415926;
	vector<Vec4> values;
	for (int i = 0; i < size; ++i)
	{
		float value = static_cast<float>(rand()) / RAND_MAX;
		value *= 180;

		double sinValue = sin(value * PI / 180);
		double cosValue = cos(value * PI / 180);

		values.emplace_back(Vec4(sinValue, cosValue, 0.0f, 0.0f));
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = values.data();
	initData.SysMemPitch = width * sizeof(Vec4);
	initData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Texture2D> texture;
	DEVICE->CreateTexture2D(&desc, &initData, texture.GetAddressOf());

	ComPtr<ID3D11ShaderResourceView> srv;
	DEVICE->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());

	_noiseSRV = srv;
}
