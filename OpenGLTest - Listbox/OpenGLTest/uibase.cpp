#include "uibase.h"

UIBase::UIBase() : 
	m_index(-1), 
	m_id(-1), 
	m_enable(true)
{
	_visible = true;
}


UIBase::~UIBase()
{
	
}

bool UIBase::OnMouseMove(int x, int y)
{
	return false;
}

bool UIBase::OnMouseDown(int x, int y)
{
	return false;
}

bool UIBase::OnMouseDownOut(int x, int y)
{
	return false;
}

bool UIBase::OnMouseUp(int x, int y)
{
	return false;
}

bool UIBase::OnMouseUpOut(int x, int y)
{
	return false;
}

bool UIBase::OnKeyDown(int keycode)
{
	return false;
}

bool UIBase::OnKeyPress(int keycode)
{
	return false;
}

void UIBase::OnMouseEnterMove(int x, int y)
{

}

void UIBase::OnMouseLeave()
{

}

void UIBase::SetEnable(bool value) {
	m_enable = value;
}

void UIBase::SetVisible(bool value)
{
	_visible = value;
	SetEnable(value);
}

void UIBase::OnUpdate(float elapsed)
{
	//...
}

void UIBase::OnDraw()
{
	//...
}