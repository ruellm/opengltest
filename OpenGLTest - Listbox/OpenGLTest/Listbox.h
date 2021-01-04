// Basic Custom Listbox UI control
// created September 11, 2018
// ruellm@yahoo.com

#pragma once

#include <vector>
#include "uibase.h"
#include "FBO.h"

class ListBoxItem : public UIBase
{
public:
	ListBoxItem();
	virtual ~ListBoxItem();

	virtual void OnUpdate(float elapsed);
	virtual void Draw(int x, int y);

	virtual void Initialize();

private:
	GLuint _texture;
};

class Listbox : public UIBase
{
public:
	Listbox();
	~Listbox();

	virtual void OnUpdate(float elapsed);
	virtual void OnDraw();

	virtual bool OnMouseMove(int x, int y);
	virtual bool OnMouseDown(int x, int y);
	virtual bool OnMouseDownOut(int x, int y);
	virtual bool OnMouseUp(int x, int y);
	virtual bool OnMouseUpOut(int x, int y);

	virtual void OnMouseEnterMove(int x, int y);
	virtual void OnMouseLeave();

	virtual void Initialize(int width, int height);

private:
	void Scroll(int y);

protected:
	virtual void InitializeContents();

	std::vector<ListBoxItem*>	_items;
	FBO							_backbuffer;
	int							_yoffset;
	int							_marginY;
	bool						_mouseDown;
	int							_cacheY;
	int							_cacheX;

	double						_baseTime;
	double						_lastTime;
	int							_baseY;
	int							_targetYOffset;
};

