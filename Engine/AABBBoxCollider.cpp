#include "pch.h"
#include "AABBBoxCollider.h"

AABBBoxCollider::AABBBoxCollider() : BaseCollider(ColliderType::AABB)
{

}

AABBBoxCollider::~AABBBoxCollider()
{

}

void AABBBoxCollider::Update()
{
	_boundingBox.Center = GetGameObject()->GetTransform()->GetPosition();
}

bool AABBBoxCollider::Intersects(Ray & ray, OUT float& distance)
{
	return _boundingBox.Intersects(ray.position, ray.direction, OUT distance);
}
