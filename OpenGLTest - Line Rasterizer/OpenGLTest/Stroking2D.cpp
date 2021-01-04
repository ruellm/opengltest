// Ported Aug 21, 2018
// from https://hypertolosana.wordpress.com/2015/03/10/efficient-webgl-stroking/
// ruellm@yahoo.com

#include "Stroking2D.h"
#include <limits>

#define STROKE_LINE_EPSILON 0.0001
#define MATH_PI 3.1416f

void GetStrokeGeometry(
	std::vector<Point2D>& points,
	struct LineStrokeAttribute attr,
	struct std::vector<Point2D>& vertices)
{
	if (points.size() < 2)
	{
		return;
	}

	enum StrokeLineJoinType join = attr.join;
	float lineWidth = (attr.width) / 2.0f;
	float miterLimit = attr.miterLimit;
	std::vector<Point2D> middlePoints;	// middle points per each line segment
	bool closed = false;

	if (points.size() == 2)
	{
		join = STROKE_JOIN_BEVEL;
		CreateTriangles(points[0], Point2D::Middle(points[0], points[1]), points[1], vertices, lineWidth, join, miterLimit);
	}
	else {
		//...
		int i = 0;
		for (int i = 0; i < points.size() - 1; i++) {
			if (i == 0) {
				middlePoints.push_back(points[0]);
			}
			else if (i == points.size() - 2) {
				middlePoints.push_back(points[points.size() - 1]);
			}
			else {
				middlePoints.push_back(Point2D::Middle(points[i], points[i + 1]));
			}
		}

		for (i = 1; i < middlePoints.size(); i++) {
			CreateTriangles(middlePoints[i - 1], points[i], middlePoints[i], vertices, lineWidth, join, miterLimit);
		}
	}

	if (!closed) {

		if (attr.cap == STROKE_LINECAP_ROUND) {

			Point2D p00 = vertices[0];
			Point2D p01 = vertices[1];
			Point2D p02 = points[1];
			Point2D p10 = vertices[vertices.size() - 1];
			Point2D p11 = vertices[vertices.size() - 3];
			Point2D p12 = points[points.size() - 2];

			CreateRoundCap(points[0], p00, p01, p02, vertices);
			CreateRoundCap(points[points.size() - 1], p10, p11, p12, vertices);

		}
		else if (attr.cap == STROKE_LINECAP_SQUARE) {

			Point2D p00 = vertices[vertices.size() - 1];
			Point2D p01 = vertices[vertices.size() - 3];

			CreateSquareCap(
				vertices[0],
				vertices[1],
				Point2D::Sub(points[0], points[1]).Normalize().ScalarMult(Point2D::Sub(points[0], vertices[0]).Length()),
				vertices);

			CreateSquareCap(
				p00,
				p01,
				Point2D::Sub(points[points.size() - 1], points[points.size() - 2]).Normalize().ScalarMult(Point2D::Sub(p01, points[points.size() - 1]).Length()),
				vertices);
		}
	}
}

void CreateTriangles(
	const Point2D& p0,
	const Point2D& p1,
	const Point2D& p2,
	struct std::vector<Point2D>& verts,
	float width,
	enum StrokeLineJoinType join,
	float miterLimit)
{
	Point2D t0 = Point2D::Sub(p1, p0);
	Point2D t2 = Point2D::Sub(p2, p1);

	t0 = t0.Perpendicular().Normalize().ScalarMult(width);
	t2 = t2.Perpendicular().Normalize().ScalarMult(width);

	// triangle composed by the 3 points if clockwise or couterclockwise.
	// if counterclockwise, we must invert the line threshold points, otherwise the intersection point
	// could be erroneous and lead to odd results.
	if (SignedArea(p0, p1, p2) > 0) {
		t0.Invert();
		t2.Invert();
	}

	Point2D* pintersect = LineIntersection(
		Point2D::Add(p0, t0), 
		Point2D::Add(p1, t0), 
		Point2D::Add(p2, t2), 
		Point2D::Add(p1, t2));

	Point2D anchor;
	float anchorLength = std::numeric_limits<float>::max();

	if (pintersect) {
		anchor = Point2D::Sub(*pintersect, p1);
		anchorLength = anchor.Length();
	}

	float dd = (anchorLength / width);// | 0;
	Point2D p0p1 = Point2D::Sub(p0, p1);
	float p0p1Length = p0p1.Length();
	Point2D p1p2 = Point2D::Sub(p1, p2);
	float p1p2Length = p1p2.Length();
	
	if (anchorLength > p0p1Length || anchorLength > p1p2Length) {

		//   1
        //   
        //   0   2
		verts.push_back(Point2D::Add(p0, t0));
		verts.push_back(Point2D::Sub(p0, t0));
		verts.push_back(Point2D::Add(p1, t0));

		//   3   5   
		//    
		//       4		           
		verts.push_back(Point2D::Sub(p0, t0));
		verts.push_back(Point2D::Add(p1, t0));
		verts.push_back(Point2D::Sub(p1, t0));

		if (join == STROKE_JOIN_ROUND) {
			CreateRoundCap(p1, Point2D::Add(p1, t0), Point2D::Add(p1, t2), p2, verts);
		}
		else if (join == STROKE_JOIN_BEVEL || (join == STROKE_JOIN_MITER && dd >= miterLimit)) {
			verts.push_back(p1);
			verts.push_back(Point2D::Add(p1, t0));
			verts.push_back(Point2D::Add(p1, t2));
		}
		else if (join == STROKE_JOIN_MITER && dd < miterLimit && pintersect) {
			verts.push_back(Point2D::Add(p1, t0));
			verts.push_back(p1);
			verts.push_back(*pintersect);

			verts.push_back(Point2D::Add(p1, t2));
			verts.push_back(p1);
			verts.push_back(*pintersect);
		}

		//     1
		//
		//     2    0
		verts.push_back(Point2D::Add(p2, t2));
		verts.push_back(Point2D::Sub(p1, t2));
		verts.push_back(Point2D::Add(p1, t2));

		//    4   5
		//
		//        3
		verts.push_back(Point2D::Add(p2, t2));
		verts.push_back(Point2D::Sub(p1, t2));
		verts.push_back(Point2D::Sub(p2, t2));
	}
	else {

		//     0   
		//
		//     1    2
		verts.push_back(Point2D::Add(p0, t0));
		verts.push_back(Point2D::Sub(p0, t0));
		verts.push_back(Point2D::Sub(p1, anchor));

		//     3   5
		//
		//         4
		verts.push_back(Point2D::Add(p0, t0));
		verts.push_back(Point2D::Sub(p1, anchor));
		verts.push_back(Point2D::Add(p1, t0));

		if (join == STROKE_JOIN_ROUND) {

			Point2D _p0 = Point2D::Add(p1, t0);
			Point2D _p1 = Point2D::Add(p1, t2);
			Point2D _p2 = Point2D::Sub(p1, anchor);

			Point2D center = p1;

			verts.push_back(_p0);
			verts.push_back(center);
			verts.push_back(_p2);

			CreateRoundCap(center, _p0, _p1, _p2, verts);

			verts.push_back(center);
			verts.push_back(_p1);
			verts.push_back(_p2);

		}
		else {

			if (join == STROKE_JOIN_BEVEL || (join == STROKE_JOIN_MITER && dd >= miterLimit)) {
				verts.push_back(Point2D::Add(p1, t0));
				verts.push_back(Point2D::Add(p1, t2));
				verts.push_back(Point2D::Sub(p1, anchor));
			}

			if (join == STROKE_JOIN_MITER && dd < miterLimit) {
				verts.push_back(*pintersect);
				verts.push_back(Point2D::Add(p1, t0));
				verts.push_back(Point2D::Add(p1, t2));
			}
		}

		//     2    0
		//
		//     1    
		verts.push_back(Point2D::Add(p2, t2));
		verts.push_back(Point2D::Sub(p1, anchor));
		verts.push_back(Point2D::Add(p1, t2));
		
		//          3
		//
		//     4    5
		verts.push_back(Point2D::Add(p2, t2));
		verts.push_back(Point2D::Sub(p1, anchor));
		verts.push_back(Point2D::Sub(p2, t2));
	}
}

float SignedArea(const Point2D& p0,
	const Point2D& p1,
	const Point2D& p2)
{
	return (p1._x - p0._x) * (p2._y - p0._y) - (p2._x - p0._x) * (p1._y - p0._y);
}

Point2D* LineIntersection(
	const Point2D& p0,
	const Point2D& p1,
	const Point2D& p2,
	const Point2D& p3)
{
	float a0 = p1._y - p0._y;
	float b0 = p0._x - p1._x;

	float a1 = p3._y - p2._y;
	float b1 = p2._x - p3._x;

	float det = a0 * b1 - a1 * b0;
	if (det > -STROKE_LINE_EPSILON && det < STROKE_LINE_EPSILON) {
		return NULL;
	}
	else {
		float c0 = a0 * p0._x + b0 * p0._y;
		float c1 = a1 * p2._x + b1 * p2._y;

		float x = (b1 * c0 - b0 * c1) / det;
		float y = (a0 * c1 - a1 * c0) / det;
		return new Point2D(x, y);
	}
}

void CreateRoundCap(const Point2D& center,
	const Point2D& _p0,
	const Point2D& _p1,
	const Point2D& nextPointInLine,
	struct std::vector<Point2D>& verts)
{
	float radius = Point2D::Sub(center, _p0).Length();

	float angle0 = atan2f((_p1._y - center._y), (_p1._x - center._x));
	float angle1 = atan2f((_p0._y - center._y), (_p0._x - center._x));

	float orgAngle0 = angle0;

	// make the round caps point in the right direction.

	// calculate minimum angle between two given angles.
	// for example: -Math.PI, Math.PI = 0, -Math.PI/2, Math.PI= Math.PI/2, etc.
	if (angle1 > angle0) {
		while (angle1 - angle0 >= MATH_PI - STROKE_LINE_EPSILON) {
			angle1 = angle1 - 2 * MATH_PI;
		}
	}
	else {
		while (angle0 - angle1 >= MATH_PI - STROKE_LINE_EPSILON) {
			angle0 = angle0 - 2 * MATH_PI;
		}
	}

	float angleDiff = angle1 - angle0;

	// for angles equal Math.PI, make the round point in the right direction.
	if (abs(angleDiff) >= MATH_PI - STROKE_LINE_EPSILON && abs(angleDiff) <= MATH_PI + STROKE_LINE_EPSILON) {
		Point2D r1 = Point2D::Sub(center, nextPointInLine);
		if (r1._x == 0) {
			if (r1._y > 0) {
				angleDiff = -angleDiff;
			}
		}
		else if (r1._x >= -STROKE_LINE_EPSILON) {
			angleDiff = -angleDiff;
		}
	}


	// calculate points, and make the cap.
	int nsegments = ((int)abs(angleDiff * radius) / 7) >> 0;
	nsegments++;

	float angleInc = angleDiff / nsegments;

	for (int i = 0; i < nsegments; i++) {
		verts.push_back(Point2D(center._x, center._y));
		verts.push_back(Point2D(
			center._x + radius * cos(orgAngle0 + angleInc * i),
			center._y + radius * sin(orgAngle0 + angleInc * i)
		));
		verts.push_back(Point2D(
			center._x + radius * cos(orgAngle0 + angleInc * (1 + i)),
			center._y + radius * sin(orgAngle0 + angleInc * (1 + i))
		));
	}
}

void CreateSquareCap(
	const Point2D& p0,
	const Point2D& p1,
	const Point2D& dir,
	struct std::vector<Point2D>& verts)
{
	verts.push_back(p0);
	verts.push_back(Point2D::Add(p0, dir));
	verts.push_back(Point2D::Add(p1, dir));

	verts.push_back(p1);
	verts.push_back(Point2D::Add(p1, dir));
	verts.push_back(p0);
}