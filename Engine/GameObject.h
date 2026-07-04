#pragma once
#include "Component.h"
class MonoBehaviour;
class Transform;
class Camera;
class MeshRenderer;
class ModelRenderer;
class ModelAnimator;
class Light;
class BaseCollider;

class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	~GameObject();

	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FixedUpdate();

	shared_ptr<Component> GetFixedComponent(ComponentType type);
	shared_ptr<Transform> GetTransform();
	shared_ptr<Camera> GetCamera();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<ModelRenderer> GetModelRenderer();
	shared_ptr<ModelAnimator> GetModelAnimator();
	shared_ptr<Light> GetLight();
	shared_ptr<BaseCollider> GetCollider();

	template <typename T>
	shared_ptr<T> GetComponent(T)
	{
		for (const shared_ptr<MonoBehaviour>& script : _scripts)
		{
			shared_ptr<T> casted = dynamic_pointer_cast<T>(script);

			if (casted != nullptr)
			{
				return casted;
			}

			return nullptr;
		}
	}

	shared_ptr<Transform> GetOrAddTransform();
	void AddComponent(shared_ptr<Component> component);

protected:
	array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components;
	vector<shared_ptr<Component>> _scripts;
};

