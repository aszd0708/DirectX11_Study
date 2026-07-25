#include "pch.h"
#include "ShadowSpotDemo.h"
#include "GameObject.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "ShadowMap.h"
#include "ModelRenderer.h"
#include "Terrain.h"
#include "Model.h"

void ShadowSpotDemo::Init()
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
		lightDesc.type = (int)LightDesc::eLightType::Directional;
		lightDesc.angle = 45.0f;
		lightDesc.range = 1.0f;
		lightDesc.position = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
		lightDesc.direction = CUR_SCENE->GetCamera()->GetTransform()->GetLook();

		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}

	CreateShadowMap();

	CreateModel();
	CreateOtherModels();
	CreateTerrain();
}

void ShadowSpotDemo::CreateShadowMap()
{
	_shadowMaps = make_shared<ShadowMapSpot>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	_shadowBuffer = make_shared<ConstantBuffer<ShadowDesc>>();
	_shadowBuffer->Create();
}

Matrix ShadowSpotDemo::GetLightView()
{
	LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
	Vec3 dir = desc.direction;
	dir.Normalize();
	return ::XMMatrixLookAtLH(desc.position, desc.position + dir, Vec3(0, 1, 0));
}

Matrix ShadowSpotDemo::GetLightProj()
{
	float fovY = ::XMConvertToRadians(XMMin(XMMax(_lightAngle, 1.0f), 170.0f));
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 1.0f, _lightRange);
}

Matrix ShadowSpotDemo::GetLightVP()
{
	return GetLightView() * GetLightProj();
}

void ShadowSpotDemo::CreateTerrain()
{
	_terrainObject = make_shared<GameObject>();
	shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Grass", L"..\\Resources\\Textures\\Terrain\\BaseColor.jpg");
	shared_ptr<Texture> heightMap = RESOURCES->Load<Texture>(L"Height", L"..\\Resources\\Textures\\Terrain\\height.png");
	shared_ptr<Shader> shader = make_shared<Shader>(L"TerrainForDemo.fx");

	_terrain = make_shared<Terrain>(shader, texture, heightMap);
	_terrainObject->AddComponent(static_cast<shared_ptr<Component>>(_terrain));
	_terrainObject->GetOrAddTransform()->SetPosition(Vec3(-20.0f, -10.0f, -20.0f));
}

void ShadowSpotDemo::CreateModel()
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
	obj->GetOrAddTransform()->SetPosition(Vec3(4.0f, 3.0f, 4.0f));
}

void ShadowSpotDemo::CreateOtherModels()
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


void ShadowSpotDemo::Update()
{
	ImGui::SliderInt("Pass", &_pass, 0, 2);

	{
		ImGui::DragFloat("LightAngle", (float*)(&_lightAngle), 0.1f, 0.1f, 170.0f);
		ImGui::DragFloat("LightRange", (float*)(&_lightRange), 1.0f, 30.f, 180.f);

		LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
		desc.type = (int)LightDesc::eLightType::Spot;
		Vec3 camPos = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
		Vec3 look = CUR_SCENE->GetCamera()->GetTransform()->GetLook();

		Vec3 target = camPos + look * 10.0f;              // 내가 보는 지점
		desc.position = _rabbitObj->GetTransform()->GetPosition() + Vec3(1.f, 1.f, 0.f);
		//Vec3 dir = target - desc.position;                 // 그 지점을 향해 내리쬐기
		//dir.Normalize();
		//desc.direction = dir;
		//
		desc.range = _lightRange;
		desc.angle = _lightAngle;

		CUR_SCENE->GetLight()->GetLight()->SetLightDesc(desc);
	}

	{
		ImGui::Begin("Shadow Debugger");
		ImGui::Image((void*)_shadowMaps->GetSRV().Get(), ImVec2(256, 256));
		ImGui::End();
	}
	/*
	{
		ImGui::Begin("Shadow Debugger Mid");
		ImGui::Image((void*)_shadowMaps->GetLayerSRV(1).Get(), ImVec2(256, 256));
		ImGui::End();
	}
	{
		ImGui::Begin("Shadow Debugger Far");
		ImGui::Image((void*)_shadowMaps->GetLayerSRV(2).Get(), ImVec2(256, 256));
		ImGui::End();
	}
	*/
}

void ShadowSpotDemo::Render()
{
	RenderShadow();
	RenderObjects();
}

void ShadowSpotDemo::RenderShadow()
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();


	_shadowMaps->ClearDepthStencilView();

	DC->OMSetRenderTargets(0, nullptr, _shadowMaps->GetDSV().Get());

	_shadowShader->PushGlobalData(GetLightView(), GetLightProj());

	for (auto& obj : _towerObjs)
	{
		obj->GetModelRenderer()->SetPass(1);
	}

	INSTANCING->Render(_towerObjs, _shadowShader);

	_rabbitObj->GetModelRenderer()->SetPass(0);
	_rabbitObj->GetModelRenderer()->Render(_shadowShader);

	_terrain->SetPass(0);
	_terrain->Render(_shadowShader);
	
	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* nullDSV = nullptr;
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void ShadowSpotDemo::RenderObjects()
{
	GRAPHICS->SetViewport(GAME->GetGameDesc().width, GAME->GetGameDesc().height);
	GRAPHICS->GetViewport().RSSetViewport();
	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	ShadowDesc shadowBuffer;
	shadowBuffer.lightVP = GetLightVP();
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	_shadowBuffer->CopyData(shadowBuffer);

	shared_ptr<Shader> rabbitShader = _rabbitObj->GetModelRenderer()->GetShader();
	rabbitShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get()); 
	rabbitShader->GetSRV("ShadowMapSpot")->SetResource(_shadowMaps->GetSRV().Get());

	shared_ptr<Shader> towerShader = _towerObjs[0]->GetModelRenderer()->GetShader();
	towerShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	towerShader->GetSRV("ShadowMapSpot")->SetResource(_shadowMaps->GetSRV().Get());

	shared_ptr<Shader> terrainShader = _terrain->GetShader();
	terrainShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	terrainShader->GetSRV("ShadowMapSpot")->SetResource(_shadowMaps->GetSRV().Get());


	for (auto& obj : _towerObjs)
	{
		obj->GetModelRenderer()->SetPass(_pass);
	}
	INSTANCING->Render(_towerObjs);

	_rabbitObj->GetModelRenderer()->SetPass(_pass);
	_rabbitObj->GetModelRenderer()->Render();

	_terrain->SetPass(_pass);
	_terrain->Render();

	rabbitShader->GetSRV("ShadowMapSpot")->SetResource(nullptr);
	towerShader->GetSRV("ShadowMapSpot")->SetResource(nullptr);
	terrainShader->GetSRV("ShadowMapSpot")->SetResource(nullptr);
}
