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
