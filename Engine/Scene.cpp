#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "BaseCollider.h"

void Scene::Start()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for(shared_ptr<GameObject> object : objects)
	{
		object->Start();
	}
}

void Scene::Update()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;
	for (shared_ptr<GameObject> object : objects)
	{
		object->Update();
	}

	// INSTANCING
	vector<shared_ptr<GameObject>> temp;
	temp.insert(temp.end(), objects.begin(), objects.end());
	INSTANCING->Render(temp);
}

void Scene::LateUpdate()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->LateUpdate();
	}
}

void Scene::Add(shared_ptr<GameObject> object)
{
	_objects.insert(object);

	if (object->GetCamera() != nullptr)
	{
		_cameras.insert(object);
	}

	if (object->GetLight() != nullptr)
	{
		_lights.insert(object);
	}
}

void Scene::Remove(shared_ptr<GameObject> object)
{
	_objects.erase(object);
	_cameras.erase(object);
	_lights.erase(object);
}

shared_ptr<GameObject> Scene::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GetCamera()->GetCamera();

	float width = GRAPHICS->GetViewport().GetWidth();
	float height = GRAPHICS->GetViewport().GetHeight();

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / width + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	unordered_set<shared_ptr<GameObject>> objects = GetObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (shared_ptr<GameObject> gameObject : objects)
	{
		if (gameObject->GetCollider() == nullptr)
		{
			continue;
		}

		// ViewSpace 에서 Ray 정의
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);
		
		Vec3 worldRayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		Vec3 worldRayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		worldRayDir.Normalize();

		Ray ray = Ray(worldRayOrigin, worldRayDir);

		float distance = 0.0f;
		if (gameObject->GetCollider()->Intersects(ray, OUT distance) == false)
		{
			continue;
		}

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	return picked;
}