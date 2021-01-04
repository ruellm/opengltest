#include "uimanager.h"
//#include "engine/event.h"

UIManager::UIManager() 
	: m_focus(NULL)
{
}

UIManager::~UIManager()
{
	m_focus = NULL;
}

void UIManager::Add(UIBase* o)
{
    int count = m_ui_list.size();
	o->SetIndex(count);
	m_ui_list.push_back(o);
}

void UIManager::Remove(UIBase* o)
{
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end();it++){
		if(*it == o){
			m_ui_list.erase(it);
			return;
		}
	}
}

void UIManager::Clear()
{
	m_ui_list.clear();
}

bool UIManager::OnMouseMove(int x, int y)
{
	UIBase* obj = NULL;

	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;
		if (!obj->_visible) continue;
		obj->OnMouseMove(x, y);

		if (x >= obj->_x && x <= obj->_x + obj->_width &&
			(y >= obj->_y && y <= obj->_y + obj->_height)) {
			obj->OnMouseEnterMove(x, y);
			return true;
		}
		else {
			obj->OnMouseLeave();
		}
	}
	return false;
}

bool UIManager::OnMouseDown(int x, int y)
{
	UIBase* obj = NULL;
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;

		if (!obj->_visible) continue;

		if (x >= obj->_x && x <= obj->_x + obj->_width &&
			(y >= obj->_y && y <= obj->_y + obj->_height)) {
			obj->OnMouseDown(x, y);
			return true;
		}
		else {
			obj->OnMouseDownOut(x, y);
		}
	}

	return false;
}

bool UIManager::OnMouseUp(int x, int y)
{
	UIBase* obj = NULL;
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;

		if (!obj->_visible) continue;

		if (x >= obj->_x && x <= obj->_x + obj->_width &&
			(y >= obj->_y && y <= obj->_y + obj->_height)) {
			obj->OnMouseUp(x, y);
			return true;
		} else {
			obj->OnMouseUpOut(x, y);
		}
	}
	return false;
}

bool UIManager::OnKeyDown(int keycode)
{
	if (m_focus) {
		return m_focus->OnKeyDown(keycode);
	}
	return false;
}

bool UIManager::OnKeyPress(int keycode)
{
	if (m_focus) {
		return m_focus->OnKeyPress(keycode);
	}
	return false;
}

void UIManager::SetFocus(UIBase* control)
{
	m_focus = control;
}

void UIManager::SetFocusIdx(int idx)
{
	UIBase* obj = NULL;
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;
		if (obj->GetIndex() == idx) {
			m_focus = obj;
			break;
		}
	}
}

UIBase* UIManager::GetByID(int id)
{
	UIBase* obj = NULL;

	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;
		if (obj->GetID() == id)
			return obj;
	}

	return NULL;
}

void UIManager::OnUpdate(float elapsed)
{
	UIBase* obj = NULL;
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;
		obj->OnUpdate(elapsed);
	}

}

void UIManager::OnDraw()
{
	UIBase* obj = NULL;
	std::vector<UIBase*>::iterator it = m_ui_list.begin();
	for(;it != m_ui_list.end(); it++) {
		obj = *it;
		if (!obj->_visible) continue;
		obj->OnDraw();
	}
}

void UIManager::OnTouchEvent(int type, int x, int y)
{
#if 0
	switch (type) {
		case EVENT_TYPE_TOUCH_DOWN:
			OnMouseDown((int) x, (int) y);
			break;
		case EVENT_TYPE_TOUCH_UP:
			OnMouseUp((int) x, (int) y);
			break;
		case EVENT_TYPE_TOUCH_MOVE:
			OnMouseMove((int) x, (int) y);
			break;
	}
#endif
}
