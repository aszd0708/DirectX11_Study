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
#include "Terrain.h"
#include "RenderObject.h"
#include "Transform.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "SphereCollider.h"

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
		_radiusConstantBuffer = make_shared<ConstantBuffer<SSAORadius>>();
		_radiusConstantBuffer->Create();
	}

	{
		float widh = GAME->GetGameDesc().width;
		float heignt = GAME->GetGameDesc().height;
		_renderTarget = make_shared<RenderTarget>(widh, heignt);
	}

	{
		float widh = GAME->GetGameDesc().width;
		float heignt = GAME->GetGameDesc().height;
		_ssaoRenderTarget = make_shared<RenderTarget>(widh, heignt);
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

	// CreateQuad For SSAO
	{

		shared_ptr<Geometry<VertexTextureNormalTangentData>> geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
		GeometryHelper::CreateQuad(geometry);
		_quadIndexBuffer = make_shared<IndexBuffer>();
		_quadIndexBuffer->Create(geometry->GetIndices());
		_quadVertexBuffer = make_shared<VertexBuffer>();
		_quadVertexBuffer->Create(geometry->GetVertices());
	}

	// CreateRandomNoiseTexture For SSAO
	{
		CreateRandomNoiseTexture();
	}

	CreateModel();
	CreateOtherModels();
	CreateTerrain();
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
		float z = (static_cast<float>(rand()) / RAND_MAX);

		Vec3 offset(x, y, z);

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
		kernel.push_back(Vec4(offset.x, offset.y, offset.z, 0.0f));
	}
	return kernel;
}

void SSAODemo::ControlSelectedObject()
{
	if(_selectedObj == nullptr) return;

	shared_ptr<Transform> transform = _selectedObj->GetTransform();
	Vec3 selectedPosition = transform->GetPosition();
	Vec3 selectedRotation = transform->GetRotation();
	Vec3 selectedSize = transform->GetScale();

	ImGui::DragFloat3("Position", (float*)(&selectedPosition), 0.1f, 0, 359.9f);
	ImGui::DragFloat3("Rotation", (float*)(&selectedRotation), 0.1f, 0, 359.9f);
	ImGui::DragFloat3("Size", (float*)(&selectedSize), 0.1f, 0, 359.9f);

	transform->SetPosition(selectedPosition);
	transform->SetRotation(selectedRotation);
	transform->SetScale(selectedSize);
}

void SSAODemo::Update()
{
	ImGui::SliderInt("Pass", &_pass, 0, 3);
	ImGui::SliderFloat("Radius", &_radius, 0, 8);
	ImGui::SliderFloat("Camera Near", &_near, 0, 8);
	ImGui::SliderFloat("Camera Far", &_far, 0, 1000.0f);

	// 오브젝트 선택
	{
		if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
		{
			int32 mouseX = INPUT->GetMousePos().x;
			int32 mouseY = INPUT->GetMousePos().y;

			//Picking
			shared_ptr<GameObject> pick = CUR_SCENE->Pick(mouseX, mouseY);
			if (pick)
			{
				_selectedObj = pick;
			}
		}
	}

	//ControlSelectedObject();
}

void SSAODemo::Render()
{
	//ID3D11ShaderResourceView* nullSRVs[16] = { nullptr };
	//DC->PSSetShaderResources(0, 16, nullSRVs);

	RenderSSAO();
	RenderBilateralBlur();
	RenderModel();

	_shader->DrawIndexed(0, _pass, _quadIndexBuffer->GetCount());

	{
		SSAORadius radiusDesc;
		radiusDesc.radius = _radius;
		radiusDesc.nearValue = _near;
		radiusDesc.farValue = _far;
		_radiusConstantBuffer->CopyData(radiusDesc);
		_shader->GetConstantBuffer("SSAORadius")->SetConstantBuffer(_radiusConstantBuffer->GetComPtr().Get());
	}
}

void SSAODemo::CreateTerrain()
{
	_terrainObject = make_shared<GameObject>();
	shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Grass", L"..\\Resources\\Textures\\Terrain\\BaseColor.jpg");
	shared_ptr<Texture> heightMap = RESOURCES->Load<Texture>(L"Height", L"..\\Resources\\Textures\\Terrain\\height.png");
	shared_ptr<Shader> shader = make_shared<Shader>(L"TerrainForDemo.fx");
	
	_terrain = make_shared<Terrain>(shader, texture, heightMap);
	_terrainObject->AddComponent(static_cast<shared_ptr<Component>>(_terrain));
	_terrainObject->GetOrAddTransform()->SetPosition(Vec3(-20.0f, -10.0f, -20.0f));
}

void SSAODemo::CreateModel()
{
	shared_ptr<Shader> shader = make_shared<Shader>(L"DefaultMeshForDemo.fx");
	shared_ptr<GameObject> obj = make_shared<GameObject>();
	_rabbitObj = obj;
	obj->GetOrAddTransform()->SetPosition(Vec3(4.0f, -3.0f, 4.0f));
	obj->GetOrAddTransform()->SetScale(Vec3(0.05f));
	{
		shared_ptr<ModelRenderer> modelRenderer = make_shared<ModelRenderer>(shader);
		obj->AddComponent(modelRenderer);

		shared_ptr<Model> model = make_shared<Model>();
		model->ReadModel(L"StanfordBunny/StanfordBunny");
		model->ReadMaterial(L"StanfordBunny/StanfordBunny");
		modelRenderer->SetModel(model);
		modelRenderer->SetPass(0);

		shared_ptr<SphereCollider> collier = make_shared<SphereCollider>();
		obj->AddComponent(collier);
	}
	CUR_SCENE->Add(obj);
}

void SSAODemo::CreateOtherModels()
{
	shared_ptr<class Model> m1 = make_shared<Model>();
	m1->ReadModel(L"Tower/Tower");
	m1->ReadMaterial(L"Tower/Tower");

	shared_ptr<Shader> shader = make_shared<Shader>(L"DefaultMeshInstanceForDemo.fx");
	for (int i = 0; i < 100; ++i)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->GetOrAddTransform()->SetPosition(Vec3(rand() % 100, -3.0f, rand() % 100));
		obj->GetOrAddTransform()->SetScale(Vec3(1.0f));
		obj->GetOrAddTransform()->SetRotation(Vec3(1.6f, 0.0f, 0.0f));
		obj->AddComponent(make_shared<ModelRenderer>(shader));
		{
			obj->GetModelRenderer()->SetModel(m1);
		}

		shared_ptr<OBBBoxCollider> collier = make_shared<OBBBoxCollider>();
		obj->AddComponent(collier);

		_towerObjs.emplace_back(obj);
		CUR_SCENE->Add(obj);
	}
}

void SSAODemo::CreateRandomNoiseTexture()
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

	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Width = width;
	desc.Height = height;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
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

	_noiseTextureSRV = srv;
}

void SSAODemo::CreateQuard()
{
	TransformDesc transformDesc;
	transformDesc.W = Matrix::CreateScale(2.0f);
	_shader->PushTransformData(transformDesc);
	_shader->PushGlobalData(Matrix::Identity, Matrix::Identity);
}

void SSAODemo::RenderSSAO()
{
	_renderTarget->ClearRenderTargetView();

	DC->OMSetRenderTargets(1, _renderTarget->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	INSTANCING->Render(_towerObjs);
	_rabbitObj->GetModelRenderer()->Render();
	_terrain->Render();

	CreateQuard();

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void SSAODemo::RenderBilateralBlur()
{
	_ssaoRenderTarget->ClearRenderTargetView();

	DC->OMSetRenderTargets(1, _ssaoRenderTarget->GetRenderTargetView().GetAddressOf(), nullptr);

	_shader->GetSRV("NormalDepthMap")->SetResource(_renderTarget->GetShaderResourceView().Get());

	CreateQuard();

	uint32 stride = _quadVertexBuffer->GetStride();
	uint32 offset = _quadVertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _quadVertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_quadIndexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	_shader->DrawIndexed(0, 1, _quadIndexBuffer->GetCount());

	ID3D11RenderTargetView* nullRTV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void SSAODemo::RenderModel()
{
	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	_shader->GetConstantBuffer("SSAOBuffer")->SetConstantBuffer(_constantBuffer->GetComPtr().Get());
	_shader->GetSRV("DiffuseMap")->SetResource(_renderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("NormalDepthMap")->SetResource(_renderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("SSAOMap")->SetResource(_ssaoRenderTarget->GetShaderResourceView().Get());
	_shader->GetSRV("NoiseTexture")->SetResource(_noiseTextureSRV.Get());

	uint32 stride = _quadVertexBuffer->GetStride();
	uint32 offset = _quadVertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _quadVertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_quadIndexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);
}