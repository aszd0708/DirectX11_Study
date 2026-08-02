#include "pch.h"
#include "PBRSphereDemo.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "Material.h"
#include "MeshRenderer.h"

void PBRSphereDemo::Init()
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
		lightDesc.type = (int)LightDesc::eLightType::Directional;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(4.0f);
		lightDesc.specular = Vec4(1.0f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		lightDesc.direction.Normalize();
		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}

	CreateSphere();
}

void PBRSphereDemo::Update()
{

}

void PBRSphereDemo::Render()
{
	_sphereObj->GetMeshRenderer()->SetPass(0);
	_sphereObj->GetMeshRenderer()->Render();
}

void PBRSphereDemo::CreateSphere()
{
	_pbrShader = make_shared<Shader>(L"PBRMeshDemo.fx");
	shared_ptr<Mesh> mesh = make_shared<Mesh>();
	mesh->CreateSphere();
	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	
	meshRenderer->SetMesh(mesh);
	shared_ptr<Texture> baseColorMap = RESOURCES->GetOrAddTexture(L"PBR_Default_BaseColor", L"..\\Resources\\Textures\\PBR\\Sample\\Default\\material_a01_BaseColor.png");
	shared_ptr<Texture> normalMap = RESOURCES->Load<Texture>(L"PBR_Default_Normal", L"..\\Resources\\Textures\\PBR\\Sample\\Default\\material_a01_Normal.png");
	shared_ptr<Texture> metallicMap = RESOURCES->Load<Texture>(L"PBR_Default_Metallic", L"..\\Resources\\Textures\\PBR\\Sample\\Default\\material_a01_Metallic.png");
	shared_ptr<Texture> roughnessMap = RESOURCES->Load<Texture>(L"PBR_Default_Roughness", L"..\\Resources\\Textures\\PBR\\Sample\\Default\\material_a01_Roughness.png");
	shared_ptr<Material> material = make_shared<Material>();
	material->SetShader(_pbrShader);
	material->SetBaseColorMap(baseColorMap);
	material->SetNormalMap(normalMap);
	material->SetMetallicMap(metallicMap);
	material->SetRoughnessMap(roughnessMap);
	meshRenderer->SetMaterial(material);
	meshRenderer->SetPass(0);

	_sphereObj = make_shared<GameObject>();
	_sphereObj->AddComponent(meshRenderer);
	_sphereObj->GetOrAddTransform();

	//CUR_SCENE->Add(_sphereObj);
}
