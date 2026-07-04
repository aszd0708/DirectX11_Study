#include "pch.h"
#include "09. MeshDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "CameraScript.h"
#include "GameObject.h"
#include "Texture.h"
#include "MeshRenderer.h"

void MeshDemo::Init()
{

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform();
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());

	/*
	//Object
	_obj = make_shared<GameObject>();
	_obj->GetOrAddTransform();
	_obj->AddComponent(make_shared<MeshRenderer>());
	{
		shared_ptr<Shader> shader = make_shared<Shader>(L"07. Normal.fx");
		_obj->GetMeshRenderer()->SetShader(shader);
	}

	RESOURCES->Init();
	{
		shared_ptr<Mesh> mesh = RESOURCES->Get<Mesh>(L"Sphere");
		_obj->GetMeshRenderer()->SetMesh(mesh);
	}
	{
		shared_ptr<Texture> texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		_obj->GetMeshRenderer()->SetTexture(texture);
	}
	*/
}

void MeshDemo::Update()
{
	_camera->Update();
	_obj->Update();
}

void MeshDemo::Render()
{

}
