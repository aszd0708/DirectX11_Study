#pragma once
#include "InstancingBuffer.h"

class GameObject;

class InstancingManager
{
	DECLARE_SINGLE(InstancingManager);

public:
	void Render(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader = nullptr);
	void ClearData();

private:
	void RenderMeshRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader = nullptr);
	void RenderModelRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader = nullptr);
	void RenderAnimRenderer(vector<shared_ptr<GameObject>>& gameObjects, shared_ptr<Shader> customShader = nullptr);

private:
	void AddData(InstanceID instanceId, InstancingData& data);

private: 
	map<InstanceID/*instancedId*/, shared_ptr<InstancingBuffer>> _buffers;
};

