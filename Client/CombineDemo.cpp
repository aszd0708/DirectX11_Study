#include "pch.h"
#include "CombineDemo.h"
#include "GameObject.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Environment.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"

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
		light->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		light->AddComponent(make_shared<Light>());

		LightDesc lightDesc;
		lightDesc.type = (int)LightDesc::eLightType::Point;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(0.4f);
		lightDesc.specular = Vec4(0.4f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		lightDesc.direction.Normalize();
		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}

	CreateSkyCube();
	CreateTerrain();
	CreateModel();
}

void CombineDemo::Update()
{
	{
		//ImGui::SliderInt("Pass", &_modelPass, 0, 5);
		ImGui::SliderInt("SkyBox", &_skyIndex, 0, _skyObjs.size() - 1);
	}

	{
		ImGui::DragFloat("LightAngle", (float*)(&_lightAngle), 0.1f, 0.1f, 359.9f);
		ImGui::DragFloat("LightRange", (float*)(&_lightRange), 1.0f, 30.f, 180.f);

		LightDesc desc = CUR_SCENE->GetLight()->GetLight()->GetLightDesc();
		desc.type = (int)LightDesc::eLightType::Point;
		Vec3 camPos = CUR_SCENE->GetCamera()->GetTransform()->GetPosition();
		Vec3 look = CUR_SCENE->GetCamera()->GetTransform()->GetLook();
		desc.direction = look;

		desc.range = _lightRange;
		desc.angle = _lightAngle;

		CUR_SCENE->GetLight()->GetLight()->SetLightDesc(desc);
	}
}

void CombineDemo::Render()
{
	shared_ptr<Environment> skyCube = _skyObjs[_skyIndex]->GetComponent<Environment>();

	for (int i = 0; i < _modelObjs.size(); ++i)
	{
		shared_ptr<GameObject> obj = _modelObjs[i];
		shared_ptr<ModelRenderer> renderer = obj->GetModelRenderer();

		skyCube->ApplyIBLToShader(renderer->GetShader());
		renderer->SetPass(_modelPass);
		renderer->Render();
	}

	{
		shared_ptr<MeshRenderer> renderer = _terrain->GetMeshRenderer();
		skyCube->ApplyIBLToShader(renderer->GetShader());
		renderer->Render();
	}

	skyCube->Render();
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
		_droidObj->GetOrAddTransform()->SetPosition(Vec3(20.0f, 5.5f, 30.0f));
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
		_towerObj->GetOrAddTransform()->SetPosition(Vec3(30.0f, 7.0f, 20.0f));
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
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Night", L"SkyBox.fx");
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		_skyObjs.emplace_back(skyObj);
	}
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Sunset", L"SkyBox.fx");
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		_skyObjs.emplace_back(skyObj);
	}
	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Dusk", L"SkyBox.fx");
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		_skyObjs.emplace_back(skyObj);
	}
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
