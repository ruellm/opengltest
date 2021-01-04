#include "Point2D.h"
#include <math.h>

Point2D::Point2D() 
	: _x(0.0f), _y(0.0f)
{
	//..
}

Point2D::Point2D(float x, float y) 
	: _x(x), _y(y)
{
	//...
}

Point2D::~Point2D()
{
}

Point2D Point2D::operator* (float f)
{
	_x *= f;
	_y *= f;

	return *this;
}

Point2D Point2D::ScalarMult(float f)
{
	_x *= f;
	_y *= f;

	return *this;
}

Point2D Point2D::Perpendicular()
{
	float tempx = _x;
	_x = -_y;
	_y = tempx;
	return *this;
}

Point2D Point2D::Invert()
{
	_x = -_x;
	_y = -_y;
	return *this;
}

Point2D Point2D::Normalize()
{
	float mod = Length();
	_x /= mod;
	_y /= mod;
	return *this;
}

float Point2D::Angle()
{
	return (_y/_x);
}
float Point2D::Length()
{
	return sqrtf((_x * _x) + (_y * _y));
}

float Point2D::Angle(Point2D p0, Point2D p1)
{
	return atan2f(p1._x - p0._x, p1._y - p0._y);
}

Point2D Point2D::Add(Point2D p0, Point2D p1)
{
	return Point2D(p0._x + p1._x, p0._y + p1._y);
}

Point2D Point2D::Sub(Point2D p1, Point2D p0)
{
	return Point2D(p1._x - p0._x, p1._y - p0._y);
}

Point2D Point2D::Middle(Point2D p0, Point2D p1)
{
	return Add(p0, p1).ScalarMult(0.5f);
}
