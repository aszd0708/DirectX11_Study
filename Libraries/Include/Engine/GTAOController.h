#pragma once
#include"AOBase.h"

class GTAOController : public AOBase
{
	struct GTAODesc
	{
		float radius;
		int maxPixelRadius;
		float thickness;
		int sliceCount;

		int stepCount;
		float projX;	// Camera::S_MatProjection._11
		float projY;	// Camera::S_MatProjection._22
		float padding;
	};

public:
	GTAOController(int width, int height);
	virtual ~GTAOController() override;

	void RenderBilateralBlur() override;
	
	GTAODesc& GetDesc() { return _desc; }

protected:
	void Init() override;


public:
	

private:
	GTAODesc _desc;
	shared_ptr<ConstantBuffer<GTAODesc>> _constantBuffer;
};

