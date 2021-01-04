#pragma once
// definition ported 
// from https://hypertolosana.wordpress.com/2015/03/10/efficient-webgl-stroking/
// Ported Aug 21, 2018
// ruellm@yahoo.com

class Point2D
{
public:
	Point2D();
	Point2D(float x, float y);
	~Point2D();
	
	Point2D operator* (float f);
	Point2D ScalarMult(float f);

	Point2D Perpendicular();
	Point2D Invert();
	Point2D Normalize();
	float Angle();
	float Length();
	
	float Angle(Point2D p0, Point2D p1);
	static Point2D Add(Point2D p0, Point2D p1);
	static Point2D Sub(Point2D p0, Point2D p1);
	static Point2D Middle(Point2D p0, Point2D p1);

	//...
	float _x;
	float _y;
};

