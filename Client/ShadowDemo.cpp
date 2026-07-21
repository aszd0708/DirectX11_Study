#include "pch.h"
#include "ShadowDemo.h"
#include "GameObject.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "ShadowMap.h"
#include "ModelRenderer.h"
#include "Terrain.h"
#include "Model.h"

void ShadowDemo::Init()
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

void ShadowDemo::CreateShadowMap()
{
	_shadowMaps = make_shared<ShadowMapDiractional>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	_shadowBuffer = make_shared<ConstantBuffer<ShadowDesc>>();
	_shadowBuffer->Create();
}

Matrix ShadowDemo::GetLightView()
{
/*
	Vec3 lightPos = CUR_SCENE->GetLight()->GetTransform()->GetPosition();
	Vec3 lightDir = CUR_SCENE->GetLight()->GetLight()->GetLightDesc().direction;

	Vec3 camPos = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
	Vec3 virtualLightPos = camPos - (lightDir * 300.0f);

	return ::XMMatrixLookAtLH(virtualLightPos, virtualLightPos + lightDir, Vec3(0, 1, 0));
*/

	LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
	Vec3 dir = desc.direction; // 고정값 또는 ImGui 슬라이더 (카메라 Look 금지)
	dir.Normalize();
	return ::XMMatrixLookAtLH(desc.position, desc.position + dir, Vec3(0, 1, 0));
}

Matrix ShadowDemo::GetLightProj(ShadowMapDiractional::eShadowMapType shadowMapType)
{
/*
	float shadowWidth = 0.0f;
	switch (shadowMapType)
	{
		case ShadowMap::eShadowMapType::Near:
		{
			shadowWidth = 30.0f;
			break;
		}

		case ShadowMap::eShadowMapType::Mid:
		{
			shadowWidth = 120;
			break;
		}

		case ShadowMap::eShadowMapType::Far:
		{
			shadowWidth = 500;
			break;
		}
	}

	float shadowHeight = shadowWidth;
	float shadowNear = 0.1f;
	float shadowFar = 1000.0f;
	return ::XMMatrixOrthographicLH(shadowWidth, shadowHeight, shadowNear, shadowFar);
*/

	float fovY = ::XMConvertToRadians(_lightAngle);
	return ::XMMatrixPerspectiveFovLH(fovY, 1.0f, 0.1f, _lightRange);
}

Matrix ShadowDemo::GetLightVP(ShadowMapDiractional::eShadowMapType shadowMapType)
{
	return GetLightView() * GetLightProj(shadowMapType);
}

void ShadowDemo::CreateTerrain()
{
	_terrainObject = make_shared<GameObject>();
	shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Grass", L"..\\Resources\\Textures\\Terrain\\BaseColor.jpg");
	shared_ptr<Texture> heightMap = RESOURCES->Load<Texture>(L"Height", L"..\\Resources\\Textures\\Terrain\\height.png");
	shared_ptr<Shader> shader = make_shared<Shader>(L"TerrainForDemo.fx");

	_terrain = make_shared<Terrain>(shader, texture, heightMap);
	_terrainObject->AddComponent(static_cast<shared_ptr<Component>>(_terrain));
	_terrainObject->GetOrAddTransform()->SetPosition(Vec3(-20.0f, -10.0f, -20.0f));
}

void ShadowDemo::CreateModel()
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

void ShadowDemo::CreateOtherModels()
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


void ShadowDemo::Update()
{
	ImGui::SliderInt("Pass", &_pass, 0, 1);

	{
		ImGui::DragFloat("LightAngle", (float*)(&_lightAngle), 0.1f, 0.1f, 359.9f);
		ImGui::DragFloat("LightRange", (float*)(&_lightRange), 1.0f, 30.f, 180.f);

		LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
		desc.type = (int)LightDesc::eLightType::Point;
		Vec3 camPos = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
		Vec3 look = CUR_SCENE->GetCamera()->GetTransform()->GetLook();

		desc.position = CUR_SCENE->GetCamera()->GetTransform()->GetPosition() + Vec3(0.f, 8.f, 0.f);
		desc.direction = look;

		desc.range = _lightRange;
		desc.angle = _lightAngle;

		CUR_SCENE->GetLight()->GetLight()->SetLightDesc(desc);
	}

	/*
	{
		ImGui::Begin("Shadow Debugger Near");
		ImGui::Image((void*)_shadowMaps->GetLayerSRV(0).Get(), ImVec2(256, 256));
		ImGui::End();
	}
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

void ShadowDemo::Render()
{
	RenderShadow();
	RenderObjects();
}

void ShadowDemo::RenderShadow()
{
	Viewport vp(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	vp.RSSetViewport();

	for (int shadowMapType = (int)ShadowMapDiractional::eShadowMapType::Near; shadowMapType < ShadowMapDiractional::eShadowMapType::MAX; ++shadowMapType)
	{
		ShadowMapDiractional::eShadowMapType type = (ShadowMapDiractional::eShadowMapType)shadowMapType;
		_shadowMaps->ClearDepthStencilView(shadowMapType);

		DC->OMSetRenderTargets(0, nullptr, _shadowMaps->GetDSV(shadowMapType).Get());

		_shadowShader->PushGlobalData(GetLightView(), GetLightProj(type));

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
	DC->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void ShadowDemo::RenderObjects()
{
	GRAPHICS->SetViewport(GAME->GetGameDesc().width, GAME->GetGameDesc().height);
	GRAPHICS->GetViewport().RSSetViewport();
	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	ShadowDesc shadowBuffer;
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Near] = GetLightVP(ShadowMapDiractional::eShadowMapType::Near);
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Mid] = GetLightVP(ShadowMapDiractional::eShadowMapType::Mid);
	shadowBuffer.lightVP[ShadowMapDiractional::eShadowMapType::Far] = GetLightVP(ShadowMapDiractional::eShadowMapType::Far);
	shadowBuffer.cascadeEnd = Vec4(15.0f, 60.0f, 300.0f, 0.0f);
	_shadowBuffer->CopyData(shadowBuffer);

	shared_ptr<Shader> rabbitShader = _rabbitObj->GetModelRenderer()->GetShader(); // 또는 저장해둔 셰이더 변수
	rabbitShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get()); 
	rabbitShader->GetSRV("ShadowMapArray")->SetResource(_shadowMaps->GetSRV().Get());

	shared_ptr<Shader> towerShader = _towerObjs[0]->GetModelRenderer()->GetShader();
	towerShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	towerShader->GetSRV("ShadowMapArray")->SetResource(_shadowMaps->GetSRV().Get());

	shared_ptr<Shader> terrainShader = _terrain->GetShader();
	terrainShader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadowBuffer->GetComPtr().Get());
	terrainShader->GetSRV("ShadowMapArray")->SetResource(_shadowMaps->GetSRV().Get());


	for (auto& obj : _towerObjs)
		obj->GetModelRenderer()->SetPass(_pass);
	INSTANCING->Render(_towerObjs);

	_rabbitObj->GetModelRenderer()->SetPass(_pass);
	_rabbitObj->GetModelRenderer()->Render();

	_terrain->SetPass(_pass);
	_terrain->Render();

	rabbitShader->GetSRV("ShadowMapArray")->SetResource(nullptr);
	towerShader->GetSRV("ShadowMapArray")->SetResource(nullptr);
	terrainShader->GetSRV("ShadowMapArray")->SetResource(nullptr);
}
