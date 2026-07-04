#include "pch.h"
#include "CollisionDemo.h"
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
#include "TextureBuffer.h"
#include "Viewport.h"
#include "SphereCollider.h"
#include "Scene.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"

void CollisionDemo::Init()
{
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

		for (int i = 0; i < 1; ++i)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->GetOrAddTransform()->SetPosition(Vec3(3.0f, 0.0f, 0.0f));
			obj->AddComponent(make_shared<MeshRenderer>());
			{
				obj->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
			}
			{
				shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Sphere");
				obj->GetMeshRenderer()->SetMesh(mesh);
				obj->GetMeshRenderer()->SetPass(0);
			}
			{
				shared_ptr<SphereCollider> collider = make_shared<SphereCollider>();
				collider->SetRadius(0.5f);
				obj->AddComponent(collider);
			}
			{
				obj->AddComponent(make_shared<MoveScript>());
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

		for (int i = 0; i < 1; ++i)
		{
			shared_ptr<GameObject> obj = make_shared<GameObject>();
			obj->GetOrAddTransform()->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
			obj->AddComponent(make_shared<MeshRenderer>());
			{
				obj->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
			}
			{
				shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Cube");
				obj->GetMeshRenderer()->SetMesh(mesh);
				obj->GetMeshRenderer()->SetPass(0);
			}
			{
				shared_ptr<AABBBoxCollider> collider = make_shared<AABBBoxCollider>();
				collider->GetBoundingBox().Extents = Vec3(0.5f);
				obj->AddComponent(collider);
			}

			CUR_SCENE->Add(obj);
		}
	}
}

void CollisionDemo::Update()
{
	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		int32 mouseX = INPUT->GetMousePos().x;
		int32 mouseY = INPUT->GetMousePos().y;

		//Picking
		shared_ptr<GameObject> pick = CUR_SCENE->Pick(mouseX, mouseY);
		if(pick)
		{
			CUR_SCENE->Remove(pick);
		}
	}
}

void CollisionDemo::Render()
{
	
}

void MoveScript::Update()
{
	Vec3 pos = GetTransform()->GetPosition();
	pos.x -= DT * 1.0f;
	GetTransform()->SetPosition(pos);
}
