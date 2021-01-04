#include "Gesture.h"

using namespace PDollarGestureRecognizer;
const int SAMPLING_RESOLUTION = 32;

Gesture::Gesture()
{
}


Gesture::~Gesture()
{
}


Gesture::Gesture(std::vector<Point>& points, std::string gestureName)
{
	std::vector<Point> out;

	Name = gestureName;

	Scale(points, Points);
	TranslateTo(Points, out, Centroid(Points));
	Resample(out, Points, SAMPLING_RESOLUTION);
}


void Gesture::Scale(const std::vector<Point>& input, std::vector<Point>& out)
{
	float minx = input[0].X, miny = input[0].Y, maxx = input[0].X, maxy = input[0].Y;
	for (int i = 0; i < input.size(); i++)
	{
		if (minx > input[i].X) minx = input[i].X;
		if (miny > input[i].Y) miny = input[i].Y;
		if (maxx < input[i].X) maxx = input[i].X;
		if (maxy < input[i].Y) maxy = input[i].Y;
	}

	out.resize(input.size());
	float scale = maxx - minx;
	if (scale < maxy - miny)
		scale = maxy - miny;

	for (int i = 0; i < input.size(); i++)
	{
		out[i] = Point((input[i].X - minx) / scale, (input[i].Y - miny) / scale, input[i].StrokeID);
	}
}

void Gesture::TranslateTo(const std::vector<Point>& input, std::vector<Point>& out, Point p)
{
	out.resize(input.size());
	for (int i = 0; i < input.size(); i++)
	{
		out[i] = (Point(input[i].X - p.X, input[i].Y - p.Y, input[i].StrokeID));
	}
}

void Gesture::Resample(const std::vector<Point>& input, std::vector<Point>& out, int n)
{
	out.resize(n);
	out[0] = Point(input[0].X, input[0].Y, input[0].StrokeID);
	int numPoints = 1;
	float I = PathLength(input) / (n - 1); // computes interval length
	float D = 0;

	for (int i = 1; i < input.size(); i++)
	{
		if (input[i].StrokeID == input[i - 1].StrokeID)
		{
			float d = Geometry::EuclideanDistance(input[i - 1], input[i]);
			if (D + d >= I)
			{
				Point firstPoint = input[i - 1];
				while (D + d >= I)
				{
					// add interpolated point
					//float t = Math.Min(Math.Max((I - D) / d, 0.0f), 1.0f);
					float max = (I - D) / d;
					if (max < 0.0f)
						max = 0.0f;

					float t = max;
					if (t > 1.0f)
						t = 1.0f;

					//if (float.IsNaN(t)) t = 0.5f;

					out[numPoints++] = Point(
						(1.0f - t) * firstPoint.X + t * input[i].X,
						(1.0f - t) * firstPoint.Y + t * input[i].Y,
						input[i].StrokeID
						);

					// update partial length
					d = D + d - I;
					D = 0;
					firstPoint = out[numPoints - 1];
				}
				D = d;
			}
			else D += d;
		}
	}

	if (numPoints == n - 1) // sometimes we fall a rounding-error short of adding the last point, so add it if so
		out[numPoints++] = Point(input[input.size() - 1].X, input[input.size() - 1].Y, input[input.size() - 1].StrokeID);
}

Point Gesture::Centroid(const std::vector<Point>& input)
{
	float cx = 0, cy = 0;
	for (int i = 0; i < input.size(); i++)
	{
		cx += input[i].X;
		cy += input[i].Y;
	}
	return Point(cx / input.size(), cy / input.size(), 0);
}

float Gesture::PathLength(const std::vector<Point>& points)
{
	float length = 0;
	for (int i = 1; i < points.size(); i++)
		if (points[i].StrokeID == points[i - 1].StrokeID)
			length += Geometry::EuclideanDistance(points[i - 1], points[i]);
	return length;
}
