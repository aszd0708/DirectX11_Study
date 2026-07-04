#include "pch.h"
#include "InstancingDemo.h"
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

void InstancingDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"19. InstancingDemo.fx");

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

		// INSTANCING
		_material = material;
	}

	for (int i = 0; i < 10000; ++i)
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

			// INSTANCING 
			_mesh = mesh;
		}

		_objs.emplace_back(obj);
	}

	RENDER->Init(_shader);

	// INSTANCING
	_instanceBuffer = make_shared<VertexBuffer>();

	for (shared_ptr<GameObject> obj : _objs)
	{
		Matrix world = obj->GetTransform()->GetWorldMatrix();
		_worlds.emplace_back(world);
	}

	_instanceBuffer->Create(_worlds, /*slot*/1);
}

void InstancingDemo::Update()
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

	_material->Update();

	//Matrix world = GetTransform()->GetWorldMatrix();
	//RENDER->PushTransformData(TransformDesc{ world });\

	_mesh->GetVertexBuffer()->PushData();
	_instanceBuffer->PushData();
	_mesh->GetIndexBuffer()->PushData();

	uint32 stride = _mesh->GetVertexBuffer()->GetStride();
	uint32 offset = _mesh->GetVertexBuffer()->GetOffset();

	//shader->DrawIndexed(0, 0, _mesh->GetIndexBuffer()->GetCount(), 0, 0);
	_shader->DrawIndexedInstanced(0, 0, _mesh->GetIndexBuffer()->GetCount(), _objs.size());
}

void InstancingDemo::Render()
{
	
}
