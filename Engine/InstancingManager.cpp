#include "pch.h"
#include "InstancingManager.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"

void InstancingManager::Render(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader)
{
	ClearData();

	RenderMeshRenderer(gameObjects, customShader);
	RenderModelRenderer(gameObjects, customShader);
	RenderAnimRenderer(gameObjects, customShader);
}

void InstancingManager::ClearData()
{
	for (pair<InstanceID, shared_ptr<InstancingBuffer>> pair : _buffers)
	{
		shared_ptr<InstancingBuffer>& buffer = pair.second;
		buffer->ClearData();
	}
}

void InstancingManager::RenderMeshRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;

	// 분류단계
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr)
		{
			continue;
		}

		const InstanceID instanceId = gameObject->GetMeshRenderer()->GetInstanceID();
		cache[instanceId].emplace_back(gameObject);
	}

	for (pair<InstanceID, vector<shared_ptr<GameObject>>> pair : cache)
	{
		const vector<shared_ptr<GameObject>>& vec = pair.second;

		/* if (vec.size() == 1)
		{
		}
		else*/
		{
			const InstanceID instanceId = pair.first;

			for (int32 i = 0; i < vec.size(); ++i)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->GetTransform()->GetWorldMatrix();

				AddData(instanceId, data);
			}

			// 마지막 그림.
			shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			vec[0]->GetMeshRenderer()->RenderInstancing(buffer);
		}
	}
}

void InstancingManager::RenderModelRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;

	// 분류단계
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetModelRenderer() == nullptr)
		{
			continue;
		}

		const InstanceID instanceId = gameObject->GetModelRenderer()->GetInstanceID();
		cache[instanceId].emplace_back(gameObject);
	}

	for (pair<InstanceID, vector<shared_ptr<GameObject>>> pair : cache)
	{
		const vector<shared_ptr<GameObject>>& vec = pair.second;

		/* if (vec.size() == 1)
		{
			vec[0]->ModelAnimator()->RenderSingle();
		}
		else*/
		{
			const InstanceID instanceId = pair.first;

			for (int32 i = 0; i < vec.size(); ++i)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->GetTransform()->GetWorldMatrix();

				AddData(instanceId, data);
			}

			// 마지막 그림.
			shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			vec[0]->GetModelRenderer()->RenderInstancing(buffer, customShader);
		}
	}
}

void InstancingManager::RenderAnimRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;

	// 분류단계
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetModelAnimator() == nullptr)
		{
			continue;
		}

		const InstanceID instanceId = gameObject->GetModelAnimator()->GetInstanceID();
		cache[instanceId].emplace_back(gameObject);
	}

	for (pair<InstanceID, vector<shared_ptr<GameObject>>> pair : cache)
	{
		shared_ptr<InstancedTweenDesc> tweenDesc = make_shared<InstancedTweenDesc>();

		const vector<shared_ptr<GameObject>>& vec = pair.second;

		/* if (vec.size() == 1)
		{
			vec[0]->ModelAnimator()->RenderSingle();
		}
		else*/
		{
			const InstanceID instanceId = pair.first;

			for (int32 i = 0; i < vec.size(); ++i)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->GetTransform()->GetWorldMatrix();

				AddData(instanceId, data);

				// INSTANCING
				gameObject->GetModelAnimator()->UpdateTweenData();
				TweenDesc& desc = gameObject->GetModelAnimator()->GetTweenDesc();
				tweenDesc->tweens[i] = desc;
			}

			//RENDER->PushTweenData(*tweenDesc.get());
			

			// 마지막 그림.
			shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			vec[0]->GetModelAnimator()->RenderInstancing(buffer);
			vec[0]->GetModelAnimator()->GetShader()->PushTweenData(*tweenDesc.get());
		}
	}
}

void InstancingManager::AddData(InstanceID instanceId, InstancingData& data)
{
	if (_buffers.find(instanceId) == _buffers.end())
	{	
		_buffers[instanceId] = make_shared<InstancingBuffer>();
	}

	_buffers[instanceId]->AddData(data);
}
