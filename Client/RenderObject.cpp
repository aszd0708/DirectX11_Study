#include "pch.h"
#include "RenderObject.h"
#include "Model.h"
#include "ModelRenderer.h"


RenderObject::RenderObject(shared_ptr<Model> model, shared_ptr<Shader> shader) : _model(model), _shader(shader)
{
	Init();
}

RenderObject::RenderObject(wstring modelPath, wstring materialPath, shared_ptr<Shader> shader) : _shader(shader)
{
	_model = make_shared<Model>();
	_model->ReadModel(modelPath);
	_model->ReadMaterial(materialPath);

	Init();
}

RenderObject::~RenderObject()
{

}

void RenderObject::Render()
{
	_modelRenderer->Render();
}

void RenderObject::Init()
{
	_modelRenderer = make_shared<ModelRenderer>(_shader);

	_modelRenderer->SetModel(_model);
	_modelRenderer->SetPass(0);
}
