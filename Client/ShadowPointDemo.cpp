#include "pch.h"
#include "ShadowPointDemo.h"
#include "GameObject.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "ShadowMap.h"
#include "ModelRenderer.h"
#include "Terrain.h"
#include "Model.h"

void ShadowPointDemo::Init()
{
	RESOURCES->Init();

	_shadowShader = make_shared<Shader>(L"ShadowMapDemo.fx");
	// Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		camera->AddComponent(make_shared<Camera>());
		camera->AddComponent(make_shared<CameraScript>());
		CUR_SCENE->Add(camera);
	}

	// Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 0.f, -5.f });
		light->AddComponent(make_shared<Light>());

		LightDesc lightDesc;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(1.0f);
		lightDesc.specular = Vec4(0.1f);

		// Point Light
		lightDesc.type = (int)LightDesc::eLightType::Point;
		lightDesc.angle = 45.0f;
		lightDesc.range = 1.0f;

		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}

	CreateShadowMap();

	CreateModel();
	CreateOtherModels();
	CreateTerrain();
}

void ShadowPointDemo::CreateShadowMap()
{
	_shadowMaps = make_shared<ShadowMapPoint>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	_shadowBuffer = make_shared<ConstantBuffer<ShadowDesc>>();
	_shadowBuffer->Create();
}

Matrix ShadowPointDemo::GetLightView(int index)
{
	LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
	Vec3 lookVector = ShadowMapPoint::LOOK_VECTOR[index];
	Vec3 upVector = ShadowMapPoint::UP_VECTOR[index];
	return ::XMMatrixLookAtLH(desc.position, desc.position + lookVector, upVector);
}

Matrix ShadowPointDemo::GetLightProj()
{
	float fovY = DirectX::XM_PIDIV2;
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 1.0f, _lightRange);
}

Matrix ShadowPointDemo::GetLightVP(int index)
{
	return GetLightView(index) * GetLightProj();
}

void ShadowPointDemo::CreateTerrain()
{
	_terrainObject = make_shared<GameObject>();
	shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Grass", L"..\\Resources\\Textures\\Terrain\\BaseColor.jpg");
	shared_ptr<Texture> heightMap = RESOURCES->Load<Texture>(L"Height", L"..\\Resources\\Textures\\Terrain\\height.png");
	shared_ptr<Shader> shader = make_shared<Shader>(L"TerrainForDemo.fx");

	_terrain = make_shared<Terrain>(shader, texture, heightMap);
	_terrainObject->AddComponent(static_cast<shared_ptr<Component>>(_terrain));
	_terrainObject->GetOrAddTransform()->SetPosition(Vec3(-20.0f, -10.0f, -20.0f));
}

void ShadowPointDemo::CreateModel()
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
	}
}

void ShadowPointDemo::CreateOtherModels()
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

		_towerObjs.emplace_back(obj);
	}
}


void ShadowPointDemo::Update()
{
	ImGui::SliderInt("Pass", &_pass, 0, 2);

	{
		ImGui::DragFloat("LightAngle", (float*)(&_lightAngle), 0.1f, 0.1f, 170.0f);
		ImGui::DragFloat("LightRange", (float*)(&_lightRange), 1.0f, 30.f, 1000.f);

		LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
		Vec3 camPos = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
		Vec3 look = CUR_SCENE->GetCamera()->GetTransform()->GetLook();

		desc.position = _rabbitObj->GetTransform()->GetPosition() + Vec3(20.0f, 1.f, 20.0f);
		desc.range = _lightRange;
		desc.angle = _lightAngle;

		CUR_SCENE->GetLight()->GetLight()->SetLightDesc(desc);
	}

	{
		ImGui::Begin("CubeMap Debugger");
		for (int i = 0; i < ShadowMapPoint::MAX_TEXTURE_COUNT; i++)
		{
			// i번째 면을 128x128 크기로 띄워봄!
			ImGui::Image((void*)_shadowMaps->GetLayerSRV(i).Get(), ImVec2(128, 128));
			if (i != 2 && i != 5) ImGui::SameLine();
		}
		ImGui::End();
	}
}

void ShadowPointDemo::Render()
{
	RenderShadow();
	RenderObjects();
}

void ShadowPointDemo::RenderShadow()
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();

	for (int i = 0; i < ShadowMapPoint::MAX_TEXTURE_COUNT; ++i)
	{
		_shadowMaps->ClearDepthStencilView(i);

		DC->OMSetRenderTargets(0, nullptr, _shadowMaps->GetDSV(i).Get());

		_shadowShader->PushGlobalData(GetLightView(i), GetLightProj());

		for (auto& obj : _towerObjs)
		{
			obj->GetModelRenderer()->SetPass(1);
		}

		INSTANCING->Render(_towerObjs, _shadowShader);

		_rabbitObj->GetModelRenderer()->SetPass(0);
		_rabbitObj->GetModelRenderer()->Render(_shadowShader);

		_terrain->SetPass(0);
		_terrain->Render(_shadowShader);
	}
	
	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* nullDSV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void ShadowPointDemo::RenderObjects()
{
	GRAPHICS->SetViewport(GAME->GetGameDesc().width, GAME->GetGameDesc().height);
	GRAPHICS->GetViewport().RSSetViewport();
	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	ShadowDesc shadowBuffer;
	
	Matrix lightProj = GetLightProj();
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	shadowBuffer.lightProjValues = Vec2(lightProj._33, lightProj._43);
	_shadowBuffer->CopyData(shadowBuffer);

	shared_ptr<Shader> rabbitShader = _rabbitObj->GetModelRenderer()->GetShader();
	rabbitShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get()); 
	rabbitShader->GetSRV("ShadowMapCubePoint")->SetResource(_shadowMaps->GetSRV().Get()); 
	{
		auto var = rabbitShader->GetSRV("ShadowMapCubePoint");
		assert(var->IsValid()); // 임시 확인
	}

	shared_ptr<Shader> towerShader = _towerObjs[0]->GetModelRenderer()->GetShader();
	towerShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	towerShader->GetSRV("ShadowMapCubePoint")->SetResource(_shadowMaps->GetSRV().Get());
	{
		auto var = towerShader->GetSRV("ShadowMapCubePoint");
		assert(var->IsValid()); // 임시 확인
		}

	shared_ptr<Shader> terrainShader = _terrain->GetShader();
	terrainShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	terrainShader->GetSRV("ShadowMapCubePoint")->SetResource(_shadowMaps->GetSRV().Get());
	{
		auto var = terrainShader->GetSRV("ShadowMapCubePoint");
		assert(var->IsValid()); // 임시 확인
	}


	for (auto& obj : _towerObjs)
	{
		obj->GetModelRenderer()->SetPass(_pass);
	}
	INSTANCING->Render(_towerObjs);

	_rabbitObj->GetModelRenderer()->SetPass(_pass);
	_rabbitObj->GetModelRenderer()->Render();

	_terrain->SetPass(_pass);
	_terrain->Render();

	rabbitShader->GetSRV("ShadowMapCubePoint")->SetResource(nullptr);
	towerShader->GetSRV("ShadowMapCubePoint")->SetResource(nullptr);
	terrainShader->GetSRV("ShadowMapCubePoint")->SetResource(nullptr);
}
