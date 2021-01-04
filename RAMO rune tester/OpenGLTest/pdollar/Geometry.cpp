#include "Geometry.h"
#include "math.h"
using namespace PDollarGestureRecognizer;

Geometry::Geometry()
{
}


Geometry::~Geometry()
{
}

/// <summary>
/// Computes the Squared Euclidean Distance between two points in 2D
/// </summary>
float Geometry::SqrEuclideanDistance(Point a, Point b)
{
	return (a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y);
}

/// <summary>
/// Computes the Euclidean Distance between two points in 2D
/// </summary>
float Geometry::EuclideanDistance(Point a, Point b)
{
	return (float)sqrt(SqrEuclideanDistance(a, b));
}