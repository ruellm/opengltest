#include <string>
#include <math.h>

#include "PointCloudRecognizer.h"

#define FLOAT_MAXVALUE 99999.0f

using namespace PDollarGestureRecognizer;

PointCloudRecognizer::PointCloudRecognizer()
{
}


PointCloudRecognizer::~PointCloudRecognizer()
{
}


std::string PointCloudRecognizer::Classify(Gesture candidate, 
	std::vector<Gesture>& trainingSet, float* distance)
{
	float minDistance = FLOAT_MAXVALUE;

	std::string gestureClass = "";
	for (int i = 0; i < trainingSet.size(); i++)
	{
		Gesture gesture = trainingSet.at(i);
		float dist = GreedyCloudMatch(candidate.Points, gesture.Points);
		if (dist < minDistance)
		{
			minDistance = dist;
			gestureClass = gesture.Name;
		}
	}
	
	*distance = minDistance;
	return gestureClass;
}

float PointCloudRecognizer::GreedyCloudMatch(std::vector<Point>& points1, std::vector<Point>& points2)
{
	int n = points1.size(); // the two clouds should have the same number of points by now
	float eps = 0.5f;       // controls the number of greedy search trials (eps is in [0..1])
	int step = (int)floor(pow(n, 1.0f - eps));
	float minDistance = FLOAT_MAXVALUE;	//float.MaxValue;

	for (int i = 0; i < n; i += step)
	{
		float dist1 = CloudDistance(points1, points2, i);   // match points1 --> points2 starting with index point i
		float dist2 = CloudDistance(points2, points1, i);   // match points2 --> points1 starting with index point i
		//minDistance = min(minDistance, Min(dist1, dist2));
		float min1 = dist1;
		if (min1 > dist2)
			min1 = dist2;
		if (minDistance > min1)
			minDistance = min1;
	}
	return minDistance;
}

float PointCloudRecognizer::CloudDistance(std::vector<Point>& points1, std::vector<Point>& points2, int startIndex)
{
	int n = points1.size();       // the two clouds should have the same number of points by now
	bool* matched = new bool[n]; // matched[i] signals whether point i from the 2nd cloud has been already matched
	//Array.Clear(matched, 0, n);   // no points are matched at the beginning
	memset(matched, 0, n);

	float sum = 0;  // computes the sum of distances between matched points (i.e., the distance between the two clouds)
	int i = startIndex;
	do
	{
		int index = -1;
		float minDistance = FLOAT_MAXVALUE;
		for (int j = 0; j < n; j++)
			if (!matched[j])
			{
				float dist = Geometry::SqrEuclideanDistance(points1[i], points2[j]);  // use squared Euclidean distance to save some processing time
				if (dist < minDistance)
				{
					minDistance = dist;
					index = j;
				}
			}
		matched[index] = true; // point index from the 2nd cloud is matched to point i from the 1st cloud
		float weight = 1.0f - ((i - startIndex + n) % n) / (1.0f * n);
		sum += weight * minDistance; // weight each distance with a confidence coefficient that decreases from 1 to 0
		i = (i + 1) % n;
	} while (i != startIndex);
	return sum;
}