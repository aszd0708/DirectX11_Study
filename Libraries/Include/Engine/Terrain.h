#pragma once
class Terrain : Component
{
public:
	Terrain(shared_ptr<Texture> heightMap, shared_ptr<Material> material);
	~Terrain() override;

public:
	void Render(shared_ptr<Shader> customShader);
	void SetPass(int32 pass) { _pass = pass; }


private:
	void Init();

private:
	shared_ptr<Texture> _heightMap;
	shared_ptr<Material> _material;
	shared_ptr<Geometry<VertexTextureData>> _geometry;
	shared_ptr<VertexBuffer> _vertexBuffer;
	shared_ptr<IndexBuffer> _indexBuffer;

	int _pass;
};

