#include "pch.h"
#include "CameraScript.h"
#include "Transform.h"

void CameraScript::Start()
{
	_prevMousePos = INPUT->GetMousePos();
}

void CameraScript::Update()
{
	float dt = TIME->GetDeltaTime();

	// Position
	{
		Vec3 pos = GetTransform()->GetPosition();

		if (INPUT->GetButton(KEY_TYPE::W))
		{
			pos += GetTransform()->GetLook() * _speed * dt;
		}
		if (INPUT->GetButton(KEY_TYPE::S))
		{
			pos -= GetTransform()->GetLook() * _speed * dt;
		}
		if (INPUT->GetButton(KEY_TYPE::A))
		{
			pos -= GetTransform()->GetRight() * _speed * dt;
		}
		if (INPUT->GetButton(KEY_TYPE::D))
		{
			pos += GetTransform()->GetRight() * _speed * dt;
		}

		GetTransform()->SetPosition(pos);
	}

	// Rotation
	{
		if (INPUT->GetButton(KEY_TYPE::Q))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.x += dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}
		if (INPUT->GetButton(KEY_TYPE::E))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.x -= dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}
		if (INPUT->GetButton(KEY_TYPE::Z))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.y += dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}
		if (INPUT->GetButton(KEY_TYPE::C))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.y -= dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}
	}

	{
		POINT mousePos = INPUT->GetMousePos();

		if (INPUT->GetButton(KEY_TYPE::RBUTTON))
		{
			float dx = static_cast<float>(mousePos.x - _prevMousePos.x);
			float dy = static_cast<float>(mousePos.y - _prevMousePos.y);

			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.y += dx * dt * 0.2f; // Yaw (좌우 회전)
			rotation.x += dy * dt * 0.2f; // Pitch (상하 회전)
			GetTransform()->SetLocalRotation(rotation);
		}

		_prevMousePos = mousePos;
	}
}
