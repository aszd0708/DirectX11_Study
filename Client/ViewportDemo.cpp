#include "pch.h"
#include "ViewportDemo.h"
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

void ViewportDemo::Init()
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
			obj->GetOrAddTransform()->SetPosition(Vec3(0.0f));
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

void ViewportDemo::Update()
{
	static float width = 800.0f;
	static float height = 600.0f;
	static float x = 0.0f;
	static float y = 0.0f;

	ImGui::InputFloat("Width", &width, 10.0f);
	ImGui::InputFloat("Height", &height, 10.0f);
	ImGui::InputFloat("X", &x, 10.0f);
	ImGui::InputFloat("Y", &y, 10.0f);

	GRAPHICS->SetViewport(width, height, x, y);

	//

	static Vec3 pos = Vec3(2,0,0);
	ImGui::InputFloat3("Pos", (float*)&pos);
	Viewport& vp = GRAPHICS->GetViewport();
	Vec3 pos2D = vp.Project(pos, Matrix::Identity, Camera::S_MatView, Camera::S_MatProjection);

	ImGui::InputFloat3("Pos2D", (float*)&pos2D);
	{
		Vec3 temp = vp.UnProject(pos2D, Matrix::Identity, Camera::S_MatView, Camera::S_MatProjection);
		ImGui::InputFloat3("Recale", (float*)&temp);
	}
}

void ViewportDemo::Render()
{
	
}