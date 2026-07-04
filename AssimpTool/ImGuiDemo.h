#pragma once
class ImGuiDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	void Test();

private:
	bool show_demo_window = true;
	bool show_another_window = true;
	Vec4 clear_color = Vec4(0.f);
};

