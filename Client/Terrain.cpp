#include "pch.h"
#include "Terrain.h"
#include "GeometryHelper.h"
#include "GameObject.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"

Terrain::Terrain(shared_ptr<Shader> shader, shared_ptr<Texture> texture, shared_ptr<Texture> heightMap)
	: _shader(shader), _texture(texture), _heightMap(heightMap)
{
	Init();
}

Terrain::~Terrain()
{

}

void Terrain::Render(shared_ptr<Shader> customShader)
{
	shared_ptr<Shader> activateShader = nullptr;
	if (customShader != nullptr)
	{
		activateShader = customShader;
		shared_ptr<GameObject> lightObj = SCENE->GetCurrentScene()->GetLight();
		if (lightObj != nullptr)
		{
			activateShader->PushLightData(lightObj->GetLight()->GetLightDesc());
		}
		else
		{
			activateShader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
		}
	}
	else
	{
		activateShader = _shader;
	}

	TransformDesc transformDesc;
	transformDesc.W = GetTransform()->GetWorldMatrix();
	activateShader->PushTransformData(transformDesc);
	activateShader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	activateShader->GetSRV("Texture0")->SetResource(_texture->GetComPtr().Get());
	
	uint32 stride = _vertexBuffer->GetStride();
	uint32 offset = _vertexBuffer->GetOffset();
	DC->IASetVertexBuffers(0, 1, _vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
	DC->IASetIndexBuffer(_indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

	activateShader->DrawIndexed(0, _pass, _indexBuffer->GetCount(), 0, 0);
}

void Terrain::Init()
{
	const int32 width = _heightMap->GetSize().x;
	const int32 height = _heightMap->GetSize().y;

	const DirectX::ScratchImage& info = _heightMap->GetInfo();
	uint8* pixelBuffer = info.GetPixels();

	_geometry = make_shared<Geometry<VertexTextureData>>();
	GeometryHelper::CreateGrid(_geometry, width, height);

	// CPU
	{
		vector<VertexTextureData>& v = const_cast<vector<VertexTextureData>&>(_geometry->GetVertices());

		for (int32 z = 0; z < height; ++z)
		{
			for (int32 x = 0; x < width; ++x)
			{
				int32 idx = width * z + x;
				uint8 height = pixelBuffer[idx] / 255.0f * 25.0f;
				v[idx].position.y = height; 
			}
		}
	}

	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());
	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());
}
