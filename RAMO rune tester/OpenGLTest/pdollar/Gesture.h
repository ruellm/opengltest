#pragma once
#include <vector>
#include <string>
#include "Geometry.h"

namespace PDollarGestureRecognizer 
{
	class Gesture
	{
	public:
		Gesture();
		Gesture(std::vector<Point>& points, std::string gestureName = "");
		~Gesture();

		std::vector<Point> Points;		// gesture points (normalized)
		std::string Name;				// gesture class

		/// <summary>
		/// Resamples the array of points into n equally-distanced points
		/// </summary>
		void Resample(const std::vector<Point>& input, std::vector<Point>& out, int n);
	
	private:									

		// <summary>
		/// Performs scale normalization with shape preservation into [0..1]x[0..1]
		// </summary>
		void Scale(const std::vector<Point>& input, std::vector<Point>& out);

		/// <summary>
		/// Translates the array of points by p
		/// </summary>
		void TranslateTo(const std::vector<Point>& input, std::vector<Point>& out, Point p);

		/// <summary>
		/// Computes the centroid for an array of points
		/// </summary>
		Point Centroid(const std::vector<Point>& input);

		/// <summary>
		/// Computes the path length for an array of points
		/// </summary>
		float PathLength(const std::vector<Point>& points);
	};
}

