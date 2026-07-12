#include "pch.h"
#include "ModelRenderer.h"
#include "Material.h"
#include "ModelMesh.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"

ModelRenderer::ModelRenderer(shared_ptr<Shader> shader) : Super(ComponentType::ModelRenderer), _shader(shader)
{
}

ModelRenderer::~ModelRenderer()
{
}

void ModelRenderer::SetModel(shared_ptr<Model> model)
{
	_model = model;

	const auto& materials = _model->GetMaterials();
	for (auto& material : materials)
	{
		material->SetShader(_shader);
	}
}

void ModelRenderer::Render(shared_ptr<Shader> customShader)
{
	if (_model == nullptr)
		return;

	shared_ptr<Shader> activateShader = nullptr;
	if (customShader != nullptr)
	{
		activateShader = customShader;
		shared_ptr<GameObject> lightObj = SCENE->GetCurrentScene()->GetLight();
		if (lightObj != nullptr)
		{
			activateShader->PushLightData(lightObj->GetLight()->GetLightDesc());
		}
		else
		{
			activateShader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
		}
	}
	else
	{
		activateShader = _shader;

		// Global Data
		activateShader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
	}

	// Light
	shared_ptr<GameObject> lightObj = SCENE->GetCurrentScene()->GetLight();
	if (lightObj != nullptr)
	{
		activateShader->PushLightData(lightObj->GetLight()->GetLightDesc());
	}

	// Bones
	BoneDesc boneDesc;
	const uint32 boneCount = _model->GetBoneCount();

	for (uint32 i = 0; i < boneCount; ++i)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.transforms[i] = bone->transform;
	}
	activateShader->PushBoneData(boneDesc);

	// Transform
	auto world = GetTransform()->GetWorldMatrix();
	activateShader->PushTransformData(TransformDesc{ world });

	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
		{
			mesh->material->Update();
		}

		// BoneIndex
		activateShader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		// IA
		mesh->vertexBuffer->PushData();
		mesh->indexBuffer->PushData();

		activateShader->DrawIndexed(0, _pass, mesh->indexBuffer->GetCount());
	}
}

void ModelRenderer::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer, shared_ptr<Shader> customShader)
{
	if (_model == nullptr)
		return;

	shared_ptr<Shader> activateShader = (customShader != nullptr) ? customShader : _shader;

	if (customShader == nullptr)
	{
		activateShader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
		shared_ptr<GameObject> lightObj = SCENE->GetCurrentScene()->GetLight();
		if (lightObj != nullptr)
		{
			activateShader->PushLightData(lightObj->GetLight()->GetLightDesc());
		}
	}
	// Bones
	BoneDesc boneDesc;
	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; ++i)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.transforms[i] = bone->transform;
	}
	activateShader->PushBoneData(boneDesc);
	// Transform
	auto world = GetTransform()->GetWorldMatrix();
	activateShader->PushTransformData(TransformDesc{ world });
	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
		{
			mesh->material->Update();
		}
		// BoneIndex
		activateShader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);
		// IA
		mesh->vertexBuffer->PushData();
		mesh->indexBuffer->PushData();
		buffer->PushData();
		activateShader->DrawIndexedInstanced(0, _pass, mesh->indexBuffer->GetCount(), buffer->GetCount());
	}
}

InstanceID ModelRenderer::GetInstanceID()
{
	return make_pair((uint64)_model.get(), (uint64)_material.get());
}