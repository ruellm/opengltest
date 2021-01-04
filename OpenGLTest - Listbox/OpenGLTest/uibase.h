#pragma once
#include "object2D.h"

class UIBase : public Object2D
{
public:
	UIBase();
	~UIBase();

	virtual void OnUpdate(float elapsed);
	virtual void OnDraw();

	virtual bool OnMouseMove(int x, int y);
	virtual bool OnMouseDown(int x, int y);
	virtual bool OnMouseDownOut(int x, int y);
	virtual bool OnMouseUp(int x, int y);
	virtual bool OnMouseUpOut(int x, int y);

	virtual void OnMouseEnterMove(int x, int y);
	virtual void OnMouseLeave();

	virtual bool OnKeyDown(int keycode);
	virtual bool OnKeyPress(int keycode);
	virtual void SetEnable(bool value);
	virtual void SetVisible(bool value);

	inline void SetIndex(int index) {
		m_index = index;
	}

	inline int GetIndex() const {
		return m_index;
	}
			
	inline int GetID() const {
		return m_id;
	}
			
	inline void SetID(int id) {
		m_id = id;
	}
			
	inline bool IsEnable() const {
		return m_enable;
	}


private:
	int 	m_index;
	int 	m_id;
	bool 	m_enable;
};

