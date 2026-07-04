#include "pch.h"
#include "MeshInstancingDemo.h"
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

void MeshInstancingDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"20. MeshInstancingDemo.fx");

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	// Material
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
		}

		_objs.emplace_back(obj);
	}

	RENDER->Init(_shader);
}

void MeshInstancingDemo::Update()
{
	_camera->Update();
	RENDER->Update();

	{
		LightDesc lightDesc;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(1.0f);
		lightDesc.specular = Vec4(0.1f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		RENDER->PushLightData(lightDesc);
	}

	/*
	for (shared_ptr<GameObject> obj : _objs)
	{
		obj->Update();
	}
	*/

	INSTANCING->Render(_objs);
}

void MeshInstancingDemo::Render()
{
	
}
