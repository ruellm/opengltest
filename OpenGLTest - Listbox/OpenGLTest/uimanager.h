#pragma once
#include "common.h"
#include "uibase.h"

class  UIManager
{
public:
	UIManager();
	~UIManager();

	void Add(UIBase* o);
	void Remove(UIBase* o);
	void Clear();
	bool OnMouseMove(int x, int y);
	bool OnMouseDown(int x, int y);
	bool OnMouseUp(int x, int y);
	bool OnKeyDown(int keycode);
	bool OnKeyPress(int keycode);
	void SetFocus(UIBase* control);
	void SetFocusIdx(int idx);
	UIBase* GetByID(int id);

	void OnUpdate(float elapsed);
	void OnDraw();

	void OnTouchEvent(int type, int x, int y);

	private:
	std::vector<UIBase*> m_ui_list;
	UIBase* m_focus;
};

