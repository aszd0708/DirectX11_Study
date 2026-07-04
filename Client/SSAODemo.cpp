#include "pch.h"
#include "SSAODemo.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "RenderTarget.h"
#include "GeometryHelper.h"
#include "ConstantBuffer.h"

void SSAODemo::Init()
{
	RESOURCES->Init();

	_shader = make_shared<Shader>(L"SSAODemo.fx");

	// 쉐이더에서 사용할 랜덤 값들
	vector<Vec4> randomValues = GenerateSSAOKernel(MAX_SSAO_POS_COUNT);
	SSAODesc desc;
	copy(randomValues.begin(), randomValues.end(), desc.positions);
	_constantBuffer = make_shared<ConstantBuffer<SSAODesc>>();
	_constantBuffer->Create();
	_constantBuffer->CopyData(desc);

	{
		float widh = GAME->GetGameDesc().width;
		float heignt = GAME->GetGameDesc().height;
		_renderTarget = make_shared<RenderTarget>(widh, heignt);
	}

	// Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		camera->AddComponent(make_shared<Camera>());
		camera->AddComponent(make_shared<CameraScript>());
		CUR_SCENE->Add(camera);
	}
	
	// Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		light->AddComponent(make_shared<Light>());

		LightDesc lightDesc;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(1.0f);
		lightDesc.specular = Vec4(0.1f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}
	
	// Model
	{
		shared_ptr<Model> model = make_shared<Model>();
		model->ReadModel(L"StanfordBunny/StanfordBunny");
		model->ReadMaterial(L"StanfordBunny/StanfordBunny");
		
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->GetOrAddTransform()->SetPosition(Vec3(0, 0, 0));
		obj->GetOrAddTransform()->SetScale(Vec3(0.05f));
		obj->AddComponent(make_shared<ModelRenderer>(_shader));
		{
			obj->GetModelRenderer()->SetModel(model);
			obj->GetModelRenderer()->SetPass(0);
		}

		_targetObj = obj;
	}

	shared_ptr<Geometry<VertexTextureNormalTangentData>> geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateQuad(geometry);
	_quadIndexBuffer = make_shared<IndexBuffer>();
	_quadIndexBuffer->Create(geometry->GetIndices());
	_quadVertexBuffer = make_shared<VertexBuffer>();
	_quadVertexBuffer->Create(geometry->GetVertices());


}

vector<Vec4> SSAODemo::GenerateSSAOKernel(int sampleCount)
{
	std::vector<Vec4> kernel;
	kernel.reserve(sampleCount);
	for (int i = 0; i < sampleCount; ++i)
	{
		// 1. rand() 결과값을 [-1.0f, 1.0f] 범위의 실수 난수로 변환
		// (rand() / RAND_MAX)는 [0.0, 1.0] 범위이며, 여기에 2를 곱하고 1을 빼서 [-1.0, 1.0]을 만듭니다.
		float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
		float y = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;

		Vec2 offset(x, y);

		// 2. 단위 원 내부로 영역 제한
		if (offset.LengthSquared() > 1.0f)
		{
			offset.Normalize();
		}
		// 3. 중심부 밀집을 위한 가중치 보정 (Scale Bias)
		float scale = static_cast<float>(i) / static_cast<float>(sampleCount);
		scale = 0.1f + (scale * scale) * 0.9f;
		offset *= scale;
		// 4. 16바이트 정렬 규칙을 준수하여 Vec4로 저장
		kernel.push_back(Vec4(offset.x, offset.y, 0.0f, 0.0f));
	}
	return kernel;
}

void SSAODemo::Update()
{
	ImGui::SliderInt("Pass", &_pass, 0, 2);
}

void SSAODemo::Render()
{
	_renderTarget->ClearRenderTargetView();	

	DC->OMSetRenderTargets(1, _renderTarget->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	_targetObj->GetModelRenderer()->Render();

	TransformDesc transformDesc;
	transformDesc.W = Matrix::CreateScale(2.0f);
	_shader->PushTransformData(transformDesc);
	_shader->PushGlobalData(Matrix::Identity, Matrix::Identity);

	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	_shader->GetConstantBuffer("SSAOBuffer")->SetConstantBuffer(_constantBuffer->GetComPtr().Get());
	_shader->GetSRV("DiffuseMap")->SetResource(_renderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("NormalDepthMap")->SetResource(_renderTarget->GetShaderResourceView().Get());

	uint32 stride = _quadVertexBuffer->GetStride();
	uint32 offset = _quadVertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _quadVertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_quadIndexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, _pass, _quadIndexBuffer->GetCount());

}
