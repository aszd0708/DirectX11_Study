#include "pch.h"
#include "CombineDemo.h"
#include "GameObject.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Environment.h"
#include "EnvironmentController.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "Shadow.h"

void CombineDemo::Init()
{
	RESOURCES->Init();

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
		light->GetOrAddTransform()->SetPosition(Vec3(20.0f, 6.0f, 30.0f));
		light->AddComponent(make_shared<Light>());
		


		LightDesc lightDesc;
		lightDesc.type = _intLightType;
		lightDesc.ambient = Vec4(1.0f);
		lightDesc.diffuse = Vec4(1.0f);
		lightDesc.specular = Vec4(1.0f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		lightDesc.direction.Normalize();
		light->GetLight()->SetLightDesc(lightDesc);
		light->GetOrAddTransform()->SetPosition(_lightPos);

		CUR_SCENE->Add(light);
		
		shared_ptr<Mesh> mesh = make_shared<Mesh>();
		mesh->CreateSphere();
		shared_ptr<MeshRenderer> renderer = make_shared<MeshRenderer>();
		shared_ptr<Texture> baseColorMap = RESOURCES->GetOrAddTexture(L"PBR_Default_BaseColor", L"..\\Resources\\PBR\\Textures\\Sample\\Default\\material_a01_BaseColor.png");
		shared_ptr<Texture> normalMap = RESOURCES->Load<Texture>(L"PBR_Default_Normal", L"..\\Resources\\PBR\\Textures\\Sample\\Default\\material_a01_Normal.png");
		shared_ptr<Texture> metallicMap = RESOURCES->Load<Texture>(L"PBR_Default_Metallic", L"..\\Resources\\PBR\\Textures\\Sample\\Default\\material_a01_Metallic.png");
		shared_ptr<Texture> roughnessMap = RESOURCES->Load<Texture>(L"PBR_Default_Roughness", L"..\\Resources\\PBR\\Textures\\Sample\\Default\\material_a01_Roughness.png");
		shared_ptr<Material> material = make_shared<Material>();
		shared_ptr<Shader> shader = make_shared<Shader>(L"PBRMeshDemo.fx");
		material->SetShader(shader);
		material->SetBaseColorMap(baseColorMap);
		material->SetNormalMap(normalMap);
		material->SetMetallicMap(metallicMap);
		material->SetRoughnessMap(roughnessMap);
		renderer->SetPass(0);
		renderer->SetMesh(mesh);
		renderer->SetMaterial(material);

		light->AddComponent(renderer);

		_shadow = make_shared<Shadow>((LightDesc::eLightType)lightDesc.type, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	}

	CreateSkyCube();
	CreateTerrain();
	CreateModel();
}

void CombineDemo::Update()
{
	{
		//ImGui::SliderInt("Pass", &_modelPass, 0, 5);
		ImGui::SliderFloat("SkyBox", &_blend, 0, _skyCubeController->GetEnvCount() + 1);
		_skyCubeController->SetBlendValue(_blend);
	}
	DebugSkyCube();
	DebugShadow();
}

void CombineDemo::Render()
{
	RenderShadow();
	RenderObjects();
}

void CombineDemo::RenderShadow()
{
	_shadow->Render(CUR_SCENE->GetLight()->GetLight(), _modelObjs);
}

void CombineDemo::RenderObjects()
{
	GRAPHICS->SetViewport(GAME->GetGameDesc().width, GAME->GetGameDesc().height);
	GRAPHICS->GetViewport().RSSetViewport();
	DC->OMSetRenderTargets(1, GRAPHICS->GetRenderTargetView().GetAddressOf(), GRAPHICS->GetDepthStencilView().Get());

	_shadow->CreateShadowBuffer(CUR_SCENE->GetLight()->GetLight());

	LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();

	string shadowResourceKey = "";
	switch (desc.type)
	{
	case (int)LightDesc::eLightType::Directional:
		shadowResourceKey = "ShadowMapArray";
		break;
	case (int)LightDesc::eLightType::Spot:
		shadowResourceKey = "ShadowMapSpot";
		break;
	case (int)LightDesc::eLightType::Point:
		shadowResourceKey = "ShadowMapCubePoint";
		break;
	}

	for (int i = 0; i < _modelObjs.size(); ++i)
	{
		shared_ptr<GameObject> obj = _modelObjs[i];
		shared_ptr<ModelRenderer> renderer = obj->GetModelRenderer();

		shared_ptr<Shader> shader = renderer->GetShader();
		shader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadow->GetShadowBuffer()->GetComPtr().Get());
		shader->GetSRV(shadowResourceKey)->SetResource(_shadow->GetShadowMap()->GetSRV().Get());

		_skyCubeController->BlendIBL(renderer->GetShader());

		renderer->SetPass(_modelPass);
		renderer->Render();
	}

	{
		shared_ptr<MeshRenderer> renderer = _terrain->GetMeshRenderer();

		shared_ptr<Shader> shader = renderer->GetShader();
		shader->GetConstantBuffer("ShadowBuffer")->SetConstantBuffer(_shadow->GetShadowBuffer()->GetComPtr().Get());
		shader->GetSRV(shadowResourceKey)->SetResource(_shadow->GetShadowMap()->GetSRV().Get());

		_skyCubeController->BlendIBL(renderer->GetShader());
		renderer->Render();
	}
	CUR_SCENE->GetLight()->GetMeshRenderer()->Render();
	_skyCubeController->Render();
}

void CombineDemo::CreateModel()
{
	shared_ptr<Shader> shader = make_shared<Shader>(L"PBRModelDemo.fx");

	{
		shared_ptr<ModelRenderer> modelRenderer = make_shared<ModelRenderer>(shader);
		shared_ptr<Model> model = make_shared<Model>();
		model->ReadModel(L"FreeDroid/FreeDroid");
		model->ReadMaterial(L"FreeDroid/FreeDroid");
		vector<shared_ptr<Material>> materials = model->GetMaterials();
		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->GetMaterialDesc().UseSeparateMetallicRoughness = 0;
		}
		modelRenderer->SetModel(model, false);

		_droidObj = make_shared<GameObject>();
		_droidObj->AddComponent(modelRenderer);
		_droidObj->GetOrAddTransform()->SetScale(Vec3(0.05f, 0.05f, 0.05f));
		_droidObj->GetOrAddTransform()->SetRotation(Vec3(1.5f, 0.0f, 0.0f));
		_droidObj->GetOrAddTransform()->SetPosition(Vec3(20.0f, 6.0f, 30.0f));
		_modelObjs.emplace_back(_droidObj);
	}
	{
		shared_ptr<ModelRenderer> modelRenderer = make_shared<ModelRenderer>(shader);
		shared_ptr<Model> model = make_shared<Model>();
		model->ReadModel(L"Tower/Tower");
		model->ReadMaterial(L"Tower/Tower");
		vector<shared_ptr<Material>> materials = model->GetMaterials();
		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->GetMaterialDesc().UseSeparateMetallicRoughness = 1;
		}
		modelRenderer->SetModel(model, true);

		_towerObj = make_shared<GameObject>();
		_towerObj->AddComponent(modelRenderer);
		_towerObj->GetOrAddTransform()->SetPosition(Vec3(30.0f, 9.0f, 20.0f));
		_towerObj->GetOrAddTransform()->SetScale(Vec3(2.0f, 2.0f, 2.0f));
		_modelObjs.emplace_back(_towerObj);
	}
	{
		shared_ptr<ModelRenderer> modelRenderer = make_shared<ModelRenderer>(shader);
		shared_ptr<Model> model = make_shared<Model>();
		model->ReadMaterial(L"Helmet/Helmet");
		model->ReadModel(L"Helmet/Helmet");
		vector<shared_ptr<Material>> materials = model->GetMaterials();
		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->GetMaterialDesc().flipUV = 0;
			materials[i]->GetMaterialDesc().UseSeparateMetallicRoughness = 1;
		}
		modelRenderer->SetModel(model, true);

		_helmetObj = make_shared<GameObject>();
		_helmetObj->AddComponent(modelRenderer);
		_helmetObj->GetOrAddTransform()->SetPosition(Vec3(30.0f, 7.0f, 40.0f));
		_helmetObj->GetOrAddTransform()->SetRotation(Vec3(1.5f, 0.0f, 0.0f));
		_helmetObj->GetOrAddTransform()->SetScale(Vec3(2.0f, 2.0f, 2.0f));
		_modelObjs.emplace_back(_helmetObj);
	}
}

void CombineDemo::CreateSkyCube()
{
	vector<shared_ptr<Environment>> skyCubes;
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Night", L"SkyBox.fx");
		Environment::IntensityDesc desc;
		desc.IBLIntensity = 0.2f;
		desc.LightIntensity = 0.8f;
		env->SetIntensityDesc(desc);
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		skyCubes.emplace_back(env);
		_skyCubeIbl.emplace_back(desc.IBLIntensity);
		_skyCubeLight.emplace_back(desc.LightIntensity);
	}
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Sunset", L"SkyBox.fx");
		Environment::IntensityDesc desc;
		desc.IBLIntensity = 0.5f;
		desc.LightIntensity = 0.5f;
		env->SetIntensityDesc(desc);
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		skyCubes.emplace_back(env);
		_skyCubeIbl.emplace_back(desc.IBLIntensity);
		_skyCubeLight.emplace_back(desc.LightIntensity);
	}
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Dusk", L"SkyBox.fx");
		Environment::IntensityDesc desc;
		desc.IBLIntensity = 0.3f;
		desc.LightIntensity = 0.7f;
		env->SetIntensityDesc(desc);
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		skyCubes.emplace_back(env);
		_skyCubeIbl.emplace_back(desc.IBLIntensity);
		_skyCubeLight.emplace_back(desc.LightIntensity);
	}


	shared_ptr<Environment> mainEnv = make_shared<Environment>();
	mainEnv->SetShader(L"SkyBox.fx");
	_skyObj = make_shared<GameObject>();
	_skyObj->GetOrAddTransform();
	_skyObj->AddComponent(mainEnv);

	_skyCubeController = make_shared<EnvironmentController>(skyCubes, mainEnv);
}

void CombineDemo::CreateTerrain()
{
	_terrain = make_shared<GameObject>();
	shared_ptr<Mesh> mesh = make_shared<Mesh>();
	shared_ptr<Texture> heightMap = RESOURCES->Load<Texture>(L"Terrain_Height", L"..\\Resources\\PBR\\Textures\\Terrain\\height.png");
	mesh->CreateTerrain(heightMap);

	shared_ptr<Material> material = make_shared<Material>();
	shared_ptr<Texture> albedoMap = RESOURCES->Load<Texture>(L"Terrain_Albedo", L"..\\Resources\\PBR\\Textures\\Terrain\\Tiles_Color.png");
	shared_ptr<Texture> normalMap = RESOURCES->Load<Texture>(L"Terrain_Normal", L"..\\Resources\\PBR\\Textures\\Terrain\\Tiles_NormalDX.png");
	shared_ptr<Texture> roughnessMap = RESOURCES->Load<Texture>(L"Terrain_Roughness", L"..\\Resources\\PBR\\Textures\\Terrain\\Tiles_Roughness.png");
	shared_ptr<Shader> shader = make_shared<Shader>(L"PBRTerrain.fx");
	material->SetBaseColorMap(albedoMap);
	material->SetNormalMap(normalMap);
	material->SetRoughnessMap(roughnessMap);
	material->SetShader(shader, true);

	shared_ptr<MeshRenderer> renderer = make_shared<MeshRenderer>();
	renderer->SetMaterial(material);
	renderer->SetMesh(mesh);
	_terrain->AddComponent(renderer);
	_terrain->GetOrAddTransform();
}

void CombineDemo::DebugSkyCube()
{
	ImGui::Begin("SkyCube Debugger");
	for (int i = 0; i < _skyCubeController->GetEnvCount(); ++i)
	{
		float ibl = _skyCubeIbl[i];
		float light = _skyCubeLight[i];

		if (ImGui::SliderFloat(("LightIntensity " + to_string(i)).c_str(), &light, 0.0f, 1.0f))
		{
			Environment::IntensityDesc newDesc;
			ibl = 1.0f - light;
			newDesc.IBLIntensity = ibl;
			newDesc.LightIntensity = light;
			_skyCubeController->SetIntensityDesc(i, newDesc);
			_skyCubeIbl[i] = ibl;
			_skyCubeLight[i] = light;
		}
		if (ImGui::SliderFloat(("IBLIntensity " + to_string(i)).c_str(), &ibl, 0.0f, 1.0f))
		{
			Environment::IntensityDesc newDesc;
			light = 1.0f - ibl;
			newDesc.IBLIntensity = ibl;
			newDesc.LightIntensity = light;
			_skyCubeController->SetIntensityDesc(i, newDesc);
			_skyCubeIbl[i] = ibl;
			_skyCubeLight[i] = light;
		}
	}
	ImGui::End();
}

void CombineDemo::DebugShadow()
{

	LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
	{
		ImGui::DragFloat("LightAngle", (float*)(&_lightAngle), 0.1f, 0.1f, 359.9f);
		ImGui::DragFloat("LightRange", (float*)(&_lightRange), 1.0f, 30.f, 180.f);
		ImGui::DragFloat3("LightPosition", (float*)(&_lightPos), 0.1f, -360.0f, 359.9f);
		ImGui::DragFloat3("LightDir", (float*)(&_lightDir), 0.1f, -1.0f, 1.0f);

		desc.range = _lightRange;
		desc.angle = _lightAngle;
		desc.position = _lightPos;
		desc.direction = _lightDir;
		desc.direction.Normalize();
		CUR_SCENE->GetLight()->GetOrAddTransform()->SetPosition(_lightPos);
	}

	{
		ImGui::DragFloat4("LightDiffuse", (float*)(&_lightDiffuse), 0.1f, 0.0f, 10.0f);
		ImGui::DragFloat4("LightSpecular", (float*)(&_lightSpecular), 0.1f, 0.0f, 10.0f);
		ImGui::DragFloat4("LightAmbient", (float*)(&_lightAmbient), 0.1f, 0.0f, 10.0f);
		desc.ambient = _lightAmbient;
		desc.diffuse = _lightDiffuse;
		desc.specular = _lightDiffuse;
	}

	{

		int prevValue = _intLightType;
		ImGui::SliderInt("LightType", (int*)(&_intLightType), 0, (int)LightDesc::eLightType::Spot);
		desc.type = _intLightType;
		if (prevValue != _intLightType)
		{
			_shadow = make_shared<Shadow>((LightDesc::eLightType)_intLightType, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		}
	}

	
	int count = 1;
	switch (desc.type)
	{
		case (int)LightDesc::eLightType::Directional:
			count = 3;
		break;
		case (int)LightDesc::eLightType::Point:
			count = ShadowMapPoint::MAX_TEXTURE_COUNT;
		break;
	}
	CUR_SCENE->GetLight()->GetLight()->SetLightDesc(desc);

	ImGui::Begin("Shadow Debugger");
	for (int i = 0; i < count; i++)
	{
		// i번째 면을 128x128 크기로 띄워봄!
		ImGui::Image((void*)_shadow->GetLayerSRV(i).Get(), ImVec2(128, 128));
		if (i != 2 && i != 5) ImGui::SameLine();
	}
	ImGui::End();
}
