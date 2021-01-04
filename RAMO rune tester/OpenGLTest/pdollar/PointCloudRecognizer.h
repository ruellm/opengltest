#pragma once

#include "Gesture.h"

namespace PDollarGestureRecognizer
{
	class PointCloudRecognizer
	{
	public:
		PointCloudRecognizer();
		~PointCloudRecognizer();

		/// <summary>
		/// Main function of the $P recognizer.
		/// Classifies a candidate gesture against a set of training samples.
		/// Returns the class of the closest neighbor in the training set.
		/// </summary>
		static std::string Classify(Gesture candidate, std::vector<Gesture>& trainingSet, float* distance);

		/// <summary>
		/// Implements greedy search for a minimum-distance matching between two point clouds
		/// </summary>
		static float GreedyCloudMatch(std::vector<Point>& points1, std::vector<Point>& points2);

		/// <summary>
		/// Computes the distance between two point clouds by performing a minimum-distance greedy matching
		/// starting with point startIndex
		/// </summary>
		static float CloudDistance(std::vector<Point>& points1, std::vector<Point>& points2, int startIndex);
	};
}

