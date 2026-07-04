#include "pch.h"
#include "OBBBoxCollider.h"

OBBBoxCollider::OBBBoxCollider() : BaseCollider(ColliderType::OBB)
{

}

OBBBoxCollider::~OBBBoxCollider()
{

}

void OBBBoxCollider::Update()
{
	_boundingOrientedBox.Center = GetGameObject()->GetTransform()->GetPosition();

	Vec3 rotation = GetGameObject()->GetTransform()->GetRotation();
	//_boundingOrientedBox.Orientation = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
}

bool OBBBoxCollider::Intersects(Ray & ray, OUT float& distance)
{
	return _boundingOrientedBox.Intersects(ray.position, ray.direction, OUT distance);
}
