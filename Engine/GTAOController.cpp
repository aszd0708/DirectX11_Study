#include "pch.h"
#include "GTAOController.h"
#include "Camera.h"

GTAOController::GTAOController(int width, int height) : AOBase(width, height, L"GTAO.fx")
{
	Init();
}

GTAOController::~GTAOController()
{

}

void GTAOController::RenderBilateralBlur()
{
	_desc.projX = Camera::S_MatProjection._11;
	_desc.projY = Camera::S_MatProjection._22;
	_constantBuffer->CopyData(_desc);

	AOBase::RenderBilateralBlur();
}

void GTAOController::Init()
{
	_desc.radius = 1.0f;
	_desc.maxPixelRadius = 300.0f;
	_desc.thickness = 1.0f;
	_desc.sliceCount = 2;
	_desc.stepCount = 4;
	_desc.projX = Camera::S_MatProjection._11;
	_desc.projY = Camera::S_MatProjection._22;

	_constantBuffer = make_shared<ConstantBuffer<GTAODesc>>();
	_constantBuffer->Create();

	_shader->GetConstantBuffer("GTAOBuffer")->SetConstantBuffer(_constantBuffer->GetComPtr().Get());
}
