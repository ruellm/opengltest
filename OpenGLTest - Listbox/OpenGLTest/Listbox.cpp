#include "Listbox.h"

#include "common.h" // temporary

//temporary
extern void Sprite2D_Draw(int x, int y, int width, int height, GLuint texture);
extern void Sprite2D_DrawRT(int x, int y, int width, int height, GLuint texture);
extern GLuint LoadTexture(const char* fnzme);

Listbox::Listbox() 
	: _yoffset(0)
	, _marginY(0)
	, _mouseDown(false)
	, _targetYOffset(0)

{
	_items.clear();
}


Listbox::~Listbox()
{
	_backbuffer.Cleanup();

	for (int i = 0; i < _items.size(); i++)
	{
		SAFE_DELETE(_items[i]);
	}

	_items.clear();
}

void Listbox::Initialize(int width, int height)
{
	_width = width;
	_height = height;

	//_backbuffer.Initialize(_width, _height);

	InitializeContents();
}

void Listbox::OnUpdate(float elapsed)
{
	float distance = _targetYOffset - _yoffset;	
	_yoffset += (distance * elapsed * 10);

	int maxLimit = (_items[0]->GetHeight() * (_items.size()-1));
	
	if (_yoffset > maxLimit)
		_targetYOffset = maxLimit;
	if (_yoffset < 0)
		_targetYOffset = 0;
}

void Listbox::OnDraw()
{
	glEnable(GL_SCISSOR_TEST);

	// in actual object x, y coordinates of a 2D object is specified, therefore we set the lower left to y+height
	// Y is lower left of the box itself
	int actualy = (SCR_HEIGHT - _y) - _height;	// on this equation Y is top left,
	glScissor(_x, actualy, _width, _height); //y is the lower left, the scissor of the viewport
	
	int startY = -_yoffset;
	for (int i = 0; i < _items.size(); i++)
	{
		_items[i]->Draw(0, startY);
		startY += (_items[i]->GetHeight() + _marginY);
	}

	glDisable(GL_SCISSOR_TEST);

}

bool Listbox::OnMouseMove(int x, int y)
{
	return true;
}

bool Listbox::OnMouseDown(int x, int y)
{
	printf("\n ############ MOUSE DOWN %d %d ############", x, y);

	_mouseDown = true;
	_cacheX = x;
	_cacheY = y;

	_baseY = y;
	_baseTime = glfwGetTime();
	_lastTime = _baseTime;

	return true;
}

bool Listbox::OnMouseDownOut(int x, int y)
{
	_mouseDown = false;
	return true;
}

bool Listbox::OnMouseUp(int x, int y)
{
	printf("\n MOUSE UP %d %d", x, y);

	if (x == _cacheX && y == _cacheY)
	{
		// item clicked
		int startY = -_yoffset;
		for (int it=0; it < _items.size(); it++) {

			int obj_x = _x;
			int obj_y = startY + _y;

			if (x >= obj_x && x <= obj_x + _items[it]->_width &&
				(y >= obj_y && y <= obj_y + _items[it]->_height)) {
				
				printf("Item #%d clicked", it);
			}

			startY += (_items[it]->GetHeight() + _marginY);
		}

	}
	else
	{
		Scroll(y);
	}

	_mouseDown = false;
	return true;
}

void Listbox::Scroll(int y)
{
	if (_mouseDown)
	{
		///...
		double currentTime = glfwGetTime();
		double diff = currentTime - _baseTime;

		if (diff < 0.25)
		{
			float length = y - _baseY;
			_targetYOffset += -(length*2);

			printf("\n################# Auto Scroll  %.2f len %2.f #####################", diff, length);
		}
		else
		{
			printf("\n NORMAL RELEASE %.2f", diff);
		}
	}
}

bool Listbox::OnMouseUpOut(int x, int y)
{
	Scroll(y);
	_mouseDown = false;
	return true;
}

void Listbox::OnMouseEnterMove(int x, int y)
{	
	if (_mouseDown)
	{
		//drag	
		double currtime = glfwGetTime();
		double timeDiff = currtime - _lastTime;

		printf("\n Mouse move %.2f", timeDiff);
		if (timeDiff > 1)	/// this value is based on calibration in desktop
		{
			_baseY = y;
			_baseTime = currtime;

			printf("\n Base Y changed %.2f", timeDiff);
		}

		float diff = _cacheY - y;
		//_yoffset += diff;
		_targetYOffset += diff;
		_cacheY = y;
		_lastTime = currtime;
		
	}
}

void Listbox::OnMouseLeave()
{}


void Listbox::InitializeContents()
{
	for (int i = 0; i < 100; i++)
	{
		ListBoxItem* item = new ListBoxItem();
		item->Initialize();
		_items.push_back(item);
	}
}

/**/
/**/
ListBoxItem::ListBoxItem() 
	: _texture(0)
{}

ListBoxItem::~ListBoxItem()
{
	if (_texture)
		glDeleteTextures(1, &_texture);
}

void ListBoxItem::Initialize()
{
	_texture = LoadTexture("./textures/smiley.jpg");
	_width = 515;
	_height = 47;
}

void ListBoxItem::OnUpdate(float elapsed)
{}

void ListBoxItem::Draw(int x, int y)
{
	Sprite2D_Draw(x, y, _width, _height, _texture);
}