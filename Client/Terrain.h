#pragma once
class Terrain : public MonoBehaviour
{
public:
	Terrain(shared_ptr<Shader> shader, shared_ptr<Texture> texture, shared_ptr<Texture> heightMap);
	virtual ~Terrain() override;

public:
	void Render();

private:
	void Init();

private:
	shared_ptr<Shader> _shader;
	shared_ptr<Texture> _texture;
	shared_ptr<Texture> _heightMap;
	shared_ptr<Geometry<VertexTextureData>> _geometry;
	shared_ptr<VertexBuffer> _vertexBuffer;
	shared_ptr<IndexBuffer> _indexBuffer;
};

