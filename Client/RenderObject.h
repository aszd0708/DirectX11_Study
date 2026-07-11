#pragma once

class Model;

class RenderObject : public MonoBehaviour
{
public:
	RenderObject(shared_ptr<Model> model, shared_ptr<Shader> shader);
	RenderObject(wstring modelPath, wstring materialPath, shared_ptr<Shader> shader);
	virtual ~RenderObject() override;

public:
	void Render();

private:
	void Init();

public:
	
private:
	shared_ptr<Model> _model;
	shared_ptr<Shader> _shader;
	shared_ptr<ModelRenderer> _modelRenderer;

	shared_ptr<VertexBuffer> _vertexBuffer;
	shared_ptr<IndexBuffer> _indexBuffer;
};

