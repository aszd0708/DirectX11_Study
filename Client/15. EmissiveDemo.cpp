#include "pch.h"
#include "15. EmissiveDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "CameraScript.h"
#include "GameObject.h"
#include "Texture.h"
#include "MeshRenderer.h"

void EmissiveDemo::Init()
{
	RESOURCES->Init();
	shared_ptr<Shader> shader = make_shared<Shader>(L"12. Lighting_Emissive.fx");
	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{0.0f, 0.0f, -10.0f});
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	//Object
	_obj = make_shared<GameObject>();
	_obj->GetOrAddTransform();
	_obj->AddComponent(make_shared<MeshRenderer>());
	{
		_obj->GetMeshRenderer()->SetShader(shader);
	}

	{
		shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Sphere");
		_obj->GetMeshRenderer()->SetMesh(mesh);
	}
	{
		shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		_obj->GetMeshRenderer()->SetTexture(texture);
	}

	// Object2
	_obj2 = make_shared<GameObject>();
	_obj2->GetOrAddTransform()->SetPosition(Vec3{0.5f, 0.0f, 2.0f});
	_obj2->AddComponent(make_shared<MeshRenderer>());
	{
		_obj2->GetMeshRenderer()->SetShader(shader);
	}

	{
		shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Cube");
		_obj2->GetMeshRenderer()->SetMesh(mesh);
	}
	{
		shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		_obj2->GetMeshRenderer()->SetTexture(texture);
	}

	RENDER->Init(shader);
	_shader = shader;
}

void EmissiveDemo::Update()
{
	_camera->Update();
	RENDER->Update();

	{
		Vec4 materialEmissive(1.0f, 0.f, 0.f, 1.f);
		_shader->GetVector("MaterialSpecular")->SetFloatVector((float*)&materialEmissive);
		_obj->Update();
	}

	{
		Vec4 materialEmissive(0.0f, 1.f, 0.f, 1.f);
		_shader->GetVector("MaterialSpecular")->SetFloatVector((float*)&materialEmissive);
		_obj2->Update();
	}

}

void EmissiveDemo::Render()
{

}
