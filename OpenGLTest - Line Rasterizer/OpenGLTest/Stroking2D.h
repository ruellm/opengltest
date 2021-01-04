#pragma once
// definition ported 
// from https://hypertolosana.wordpress.com/2015/03/10/efficient-webgl-stroking/
// Ported Aug 21, 2018
// Ruell Magpayo <ruellm@yahoo.com>
// TODO: wrap this in namespace to avoid clash

#include <vector>
#include "Point2D.h"

enum StrokeLineCapType
{
	STROKE_LINECAP_ROUND = 1,
	STROKE_LINECAP_SQUARE = 2,
	STROKE_LINECAP_BUTT = 0
};

enum StrokeLineJoinType
{
	STROKE_JOIN_ROUND = 0,
	STROKE_JOIN_BEVEL = 1,
	STROKE_JOIN_MITER = 2
};

struct LineStrokeAttribute
{
	float width;
	enum StrokeLineCapType cap;
	enum StrokeLineJoinType join;
	float miterLimit;
};

void GetStrokeGeometry(
	std::vector<Point2D>& points,
	struct LineStrokeAttribute attr,
	struct std::vector<Point2D>& vertices);

void CreateTriangles(
	const Point2D& p0, 
	const Point2D& p1,
	const Point2D& p2,
	struct std::vector<Point2D>& vertices,
	float width,
	enum StrokeLineJoinType join,
	float miterLimit);

float SignedArea(const Point2D& p0,
	const Point2D& p1,
	const Point2D& p2);

Point2D* LineIntersection(
	const Point2D& p0, 
	const Point2D& p1,
	const Point2D& p2,
	const Point2D& p3);

void CreateRoundCap(const Point2D& center,
	const Point2D& _p0,
	const Point2D& _p1,
	const Point2D& nextPointInLine, 
	struct std::vector<Point2D>& verts);

void CreateSquareCap(
	const Point2D& p0,
	const Point2D& p1,
	const Point2D& dir,
	struct std::vector<Point2D>& verts);
