#include "pch.h"
#include "Mesh.h"
#include "GeometryHelper.h"

Mesh::Mesh()  : Super(ResourceType::Mesh)
{

}

Mesh::~Mesh()
{

}

void Mesh::CreateQuad()
{
	_geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateQuad(_geometry);
	CreateBuffers();
}

void Mesh::CreateCube()
{
	_geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateCube(_geometry);
	CreateBuffers();
}

void Mesh::CreateGrid(int32 sizeX, int32 sizeZ)
{
	_geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateGrid(_geometry, sizeX, sizeZ);
	CreateBuffers();
}

void Mesh::CreateSphere()
{
	_geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateSphere(_geometry);
	CreateBuffers();
}

void Mesh::CreateTerrain(shared_ptr<Texture> heightMap)
{
	const int32 width = heightMap->GetSize().x;
	const int32 height = heightMap->GetSize().y;

	const DirectX::ScratchImage& info = heightMap->GetInfo();
	uint8* pixelBuffer = info.GetPixels();

	_geometry = make_shared<Geometry<VertexTextureNormalTangentData>>();
	GeometryHelper::CreateGrid(_geometry, width, height);

	// CPU
	{
		vector<VertexTextureNormalTangentData>& v = const_cast<vector<VertexTextureNormalTangentData>&>(_geometry->GetVertices());

		for (int32 z = 0; z < height; ++z)
		{
			for (int32 x = 0; x < width; ++x)
			{
				int32 idx = width * z + x;
				uint8 height = pixelBuffer[idx] / 255.0f * 25.0f;
				v[idx].position.y = height;
			}
		}

		for (int32 z = 0; z < height; ++z)
		{
			for (int32 x = 0; x < width; ++x)
			{
				int32 idx = width * z + x;
				float hL = v[max(x - 1, 0) + z * width].position.y;
				float hR = v[min(x + 1, width - 1) + z * width].position.y;
				float hD = v[x + max(z - 1, 0) * width].position.y;
				float hU = v[x + min(z + 1, height - 1) * width].position.y;

				Vec3 n = Vec3(hL - hR, 2.0f, hD - hU);
				n.Normalize();
				v[idx].normal = n;
				v[idx].tangent = Vec3(2.0f, hR - hL, 0.0f);  // ∂P/∂x
				v[idx].tangent.Normalize();
			}
		}
	}

	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());
	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());
}

void Mesh::CreateBuffers()
{
	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_geometry->GetVertices());
	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_geometry->GetIndices());
}
