#include "pch.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Game.h"
#include "Shader.h"
#include "Material.h"
#include "Light.h"

MeshRenderer::MeshRenderer() : Super(ComponentType::MeshRenderer)
{

}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer)
{
	if(_mesh == nullptr || _material == nullptr) return;

	shared_ptr<Shader> shader = _material->GetShader();
	if(shader == nullptr) return;

	// Global Data
	shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	// Light
	shared_ptr<GameObject> lightObj = SCENE->GetCurrentScene()->GetLight();
	if (lightObj != nullptr)
	{
		shader->PushLightData(lightObj->GetLight()->GetLightDesc());
	}

	_material->Update();

	// IA
	_mesh->GetVertexBuffer()->PushData();
	_mesh->GetIndexBuffer()->PushData();

	buffer->PushData();

	shader->DrawIndexedInstanced(0, _pass, _mesh->GetIndexBuffer()->GetCount(), buffer->GetCount());
}

InstanceID MeshRenderer::GetInstanceID()
{
	return make_pair((uint64)_mesh.get(), (uint64)_material.get());
}
