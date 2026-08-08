#include "pch.h"
#include "PBRModelDemo.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "Environment.h"
#include "GameObject.h"
#include "ModelAnimator.h"

void PBRModelDemo::Init()
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
		lightDesc.diffuse = Vec4(10.0f);
		lightDesc.specular = Vec4(1.0f);
		lightDesc.direction = Vec3(1.0f, 0.0f, 1.0f);
		lightDesc.direction.Normalize();
		light->GetLight()->SetLightDesc(lightDesc);

		CUR_SCENE->Add(light);
	}

	{
		shared_ptr<GameObject> skyObj = make_shared<GameObject>();
		shared_ptr<Environment> env = make_shared<Environment>();
		env->LoadHDRMap(L"Dusk", L"SkyBox.fx");
		skyObj->AddComponent(env);
		skyObj->GetOrAddTransform();
		_skyObjs.emplace_back(skyObj);
	}
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

	CreateModel();
}

void PBRModelDemo::Update()
{
	{
		_skyObjs[_skyBoxIndex]->Update();
	}

	shared_ptr<ModelRenderer> modelRenderer = _modelObj->GetModelRenderer();
	{
		ImGui::SliderInt("Pass", &_pass, 0, 5);
		ImGui::SliderInt("SkyBox", &_skyBoxIndex, 0, _skyObjs.size() - 1);

		//modelRenderer->GetTweenDesc().next.animIndex = _animationIndex;
		//ImGui::SliderInt("Animation", &_animationIndex, 0, 2);
	}
	//_modelObj->GetModelAnimator()->Update();
	//_modelObj->GetModelAnimator()->UpdateTweenData();
   
}

void PBRModelDemo::Render()
{
	shared_ptr<ModelRenderer> modelRenderer = _modelObj->GetModelRenderer();
	modelRenderer->SetPass(_pass);

	shared_ptr<Environment> env = _skyObjs[_skyBoxIndex]->GetComponent<Environment>();
	shared_ptr<Shader> envShader = modelRenderer->GetShader();
	envShader->GetSRV("IrradianceMap")->SetResource(env->GetIrradianceMap()->GetSRV().Get());
	envShader->GetSRV("PrefilteredMap")->SetResource(env->GetPrefilteredMap()->GetSRV().Get());
	envShader->GetSRV("BRDFLUT")->SetResource(env->GetBRDFMap()->GetSRV().Get());

	modelRenderer->Render();
	env->Render();
}

void PBRModelDemo::CreateModel()
{
	_pbrShader = make_shared<Shader>(L"PBRModelDemo.fx");

	shared_ptr<ModelRenderer> modelRenderer = make_shared<ModelRenderer>(_pbrShader);
	shared_ptr<Model> model = make_shared<Model>();
	model->ReadModel(L"FreeDroid/FreeDroid");
	model->ReadMaterial(L"FreeDroid/FreeDroid");
	//model->ReadAnimation(L"FreeDroid/Idle");
	//model->ReadAnimation(L"FreeDroid/Run");
	//model->ReadAnimation(L"FreeDroid/Slash");
	modelRenderer->SetModel(model);

	_modelObj = make_shared<GameObject>();
	_modelObj->AddComponent(modelRenderer);
	_modelObj->GetOrAddTransform()->SetScale(Vec3(0.1f, 0.1f, 0.1f));
	_modelObj->GetOrAddTransform()->SetRotation(Vec3(1.5f, 0.0f, 0.0f));
	//CUR_SCENE->Add(_modelObj);
}
