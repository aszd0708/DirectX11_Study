#include "pch.h"
#include "SceneDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "GameObject.h"
#include "CameraScript.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"
#include "Light.h"

void SceneDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"23. RenderDemo.fx");

	// Camera
	{
		shared_ptr<GameObject>camera = make_shared<GameObject>();
		camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		camera->AddComponent(make_shared<Camera>());
		camera->AddComponent(make_shared<CameraScript>());

		CUR_SCENE->Add(camera);
	}

	// Light
	{
		shared_ptr<GameObject>light = make_shared<GameObject>();
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

	// Animation
	{
		shared_ptr<Model> model = make_shared<Model>();
		model->ReadModel(L"Kachujin/Kachujin");
		model->ReadMaterial(L"Kachujin/Kachujin");
		model->ReadAnimation(L"Kachujin/Idle");
		model->ReadAnimation(L"Kachujin/Run");
		model->ReadAnimation(L"Kachujin/Slash");

		for (int i = 0; i < 100; ++i)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->GetOrAddTransform()->SetPosition(Vec3(rand() % 100, 0, rand() % 100));
			obj->GetOrAddTransform()->SetScale(Vec3(0.01f));
			obj->AddComponent(make_shared<ModelAnimator>(_shader));
			{
				obj->GetModelAnimator()->SetModel(model);
				obj->GetModelAnimator()->SetPass(2);
			}
			CUR_SCENE->Add(obj);
		}
	}

	// Model
	{
		shared_ptr<class Model> model = make_shared<Model>();
		model->ReadModel(L"Tower/Tower");
		model->ReadMaterial(L"Tower/Tower");

		for (int i = 0; i < 100; ++i)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->GetOrAddTransform()->SetPosition(Vec3(rand() % 100, 0, rand() % 100));
			obj->GetOrAddTransform()->SetScale(Vec3(0.01f));
			obj->AddComponent(make_shared<ModelRenderer>(_shader));
			{
				obj->GetModelRenderer()->SetModel(model);
				obj->GetModelRenderer()->SetPass(1);
			}
			CUR_SCENE->Add(obj);
		}
	}


	// Mesh
	{
		{
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(_shader);
			shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
			material->SetDiffuseMap(texture);
			MaterialDesc& desc = material->GetMaterialDesc();
			desc.ambient = Vec4(1.0f);
			desc.diffuse = Vec4(1.0f);
			desc.specular = Vec4(1.0f);
			RESOURCES->Add(L"Veigar", material);
		}

		for (int i = 0; i < 500; ++i)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->GetOrAddTransform()->SetPosition(Vec3(rand() % 100, 0, rand() % 100));
			obj->AddComponent(make_shared<MeshRenderer>());
			{
				obj->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
			}
			{
				shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Sphere");
				obj->GetMeshRenderer()->SetMesh(mesh);
				obj->GetMeshRenderer()->SetPass(0);
			}

			CUR_SCENE->Add(obj);
		}
	}
}

void SceneDemo::Update()
{
}

void SceneDemo::Render()
{
	
}
