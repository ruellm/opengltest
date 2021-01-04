#pragma once

namespace PDollarGestureRecognizer
{
	struct Point
	{
		float X;
		float Y;
		int StrokeID;
		
		Point() {
			X = 0;
			Y = 0;
			StrokeID = 0;
		}

		Point(float x, float y, int strokeId) {
			X = x;
			Y = y;
			StrokeID = strokeId;
		}
	};

	class Geometry
	{
	public:
		Geometry();
		~Geometry();

		static float SqrEuclideanDistance(Point a, Point b);
		static float EuclideanDistance(Point a, Point b);
	};
}
