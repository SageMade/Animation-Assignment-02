/*
	Authors:
	Shawn M.   - 100412327
	Shaun M.   - 100642799
	Daniel M.  - 100552012
	Stephen R. - 100458273
	Paul P.    - 100656910
	Selina D.  - 100558926
*/
#pragma once

#include <vector>

template <typename T>
struct SpeedControlTableRow
{
	int segment;
	float tValue;
	float arcLength;
	float arcLengthNormalized;
	T sampleValue;
};

template <typename T>
class KeyframeController
{
private:
	unsigned int m_pCurrentKeyframe;
	unsigned int m_pNextKeyframe;
	std::vector<T> m_pKeys;
	float m_pKeyLocalTime;

	std::vector< SpeedControlTableRow<T> > m_pSpeedControlLookUpTable;

public:
	KeyframeController()
		: m_pCurrentKeyframe(0),
		m_pNextKeyframe(1),
		m_pKeyLocalTime(0.0f),
		paused(false),
		loops(true),
		doesSpeedControl(false),
		curvePlaySpeed(1.0f)
	{

	}

	void calculateLookupTable(int numSamples)
	{
		if (m_pKeys.size() > 1) // Make sure we have enough points
		{
			m_pSpeedControlLookUpTable.clear(); // clear existing table

			float timeStep = 1.0f / numSamples; // This controls the interval at which we will sample the segment

			// Create table and compute segment, t value and sample columns of table

			for (unsigned int i = 0; i < m_pKeys.size() - 1; i++) // loop through each segment
			{
				for (float j = 0.0f; j <= 1.0f; j += timeStep) // iterate through each sample on the current segment
				{
					// Todo:
					// Create a new SpeedControlTableRow and fill it in with the appropriate data
					SpeedControlTableRow<T> row;
					row.segment = i;
					row.tValue  = j;

					row.sampleValue = Math::lerp(m_pKeys[i], m_pKeys[i + 1], j); // For this lab, we'll use lerp. But this exact algorithm works for catmull or bezier too.

					m_pSpeedControlLookUpTable.push_back(row);
				}
			}

			// Calculate arc length column of table
			int numEntries = m_pSpeedControlLookUpTable.size();

			if (numEntries == 0) // if you did the above loop correctly, this shouldnt happen
			{
				std::cout << "Failed to create look up table. " << std::endl;
				return;
			}

			// Initialize first row of table
			// Remember the struct has no ctor so we need to make sure to set everything manually
			// Note: the slides refer "arcLength" as "distance on curve"
			m_pSpeedControlLookUpTable[0].arcLength = 0.0f;
			m_pSpeedControlLookUpTable[0].arcLengthNormalized = 0.0f;

			// Loop through each point in the table and calculate the distance from the beginning of the path
			for (int i = 1; i < numEntries; i++) 
			{
				// distance = length(current sample value - previous sample value)
				float distance = length(m_pSpeedControlLookUpTable[i].sampleValue - m_pSpeedControlLookUpTable[i - 1].sampleValue);

				// m_pSpeedControlLookUpTable[i].arcLength = distance + previous sample's distance on curve
				m_pSpeedControlLookUpTable[i].arcLength = distance + m_pSpeedControlLookUpTable[i - 1].arcLength;
			}

			// Normalize the curve
			// This means 0 will be at the start of the path, and 1 will be at the end of the entire path
			float totalCurveLength = m_pSpeedControlLookUpTable[numEntries - 1].arcLength; // length of the path = distance the last sample is from the beginning

			// Normalize each sample
			// Loop through each entry in the table
			// Set "ArcLengthNormalized" to sample's distance on curve divided by total length of curve

			for (int i = 1; i < numEntries; i++)
			{
				m_pSpeedControlLookUpTable[i].arcLengthNormalized = m_pSpeedControlLookUpTable[i].arcLength / totalCurveLength;
			}
		}
	}

	T speedControlledUpdate(float dt)
	{
		// key local time is now from start of curve to end of curve, instead of per segment

		m_pKeyLocalTime += (dt / curvePlaySpeed); // control playback speed

		updateSegmentIndices();

		// look up values from table

		// Loop through each row in the table
		for (unsigned int i = 1; i < m_pSpeedControlLookUpTable.size(); i++)
		{
			// Find the first sample who's distance is >= m_pKeyLocalTime
			if (m_pSpeedControlLookUpTable[i].arcLengthNormalized >= m_pKeyLocalTime)
			{
				// calculate t value
				float arc0 = m_pSpeedControlLookUpTable[i - 1].arcLengthNormalized; //previous sample's normalized distance
				float arc1 = m_pSpeedControlLookUpTable[i].arcLengthNormalized;     // current sample's normalized distance
				float tVal = Math::invLerp(m_pKeyLocalTime, arc0, arc1); // "inverse lerp" i.e. given 3 points, solve the tValue

				// calculate intermediate table
				T sample0 = m_pSpeedControlLookUpTable[i - 1].sampleValue; //previous sample value
				T sample1 = m_pSpeedControlLookUpTable[i].sampleValue;     //current sample value

				return Math::lerp(sample0, sample1, tVal);
			}
		}

		return T(); // if lookup table is set up correctly, this should never trigger
	}	

	T update(float dt)
	{
		if (m_pKeys.size() < 2)
			return T();	

		if (doesSpeedControl)
			return speedControlledUpdate(dt);
		else
			return noSpeedControlUpdate(dt);
	}

	void updateSegmentIndices()
	{
		if (m_pKeyLocalTime >= 1.0f)
		{
			m_pKeyLocalTime = 0.0f;
			m_pCurrentKeyframe++;
			m_pNextKeyframe++;

			// If we're at the end of the animation, jump back to beginning
			// Note: you can add additional logic here to handle end of animation behaviour
			// such as: ping-ponging (playing in reverse back to beginning), closed loop, etc.
			if (m_pNextKeyframe >= m_pKeys.size())
			{
				m_pCurrentKeyframe = 0;
				m_pNextKeyframe = 1;
			}
		}
	}

	T noSpeedControlUpdate(float dt)
	{
		m_pKeyLocalTime += dt;

		updateSegmentIndices();

		T p0 = m_pKeys[m_pCurrentKeyframe];
		T p1 = m_pKeys[m_pNextKeyframe];

		return Math::lerp<T>(p0, p1, m_pKeyLocalTime);
	}

	void addKey(T key)
	{
		m_pKeys.push_back(key);
	}

	void setKey(unsigned int idx, T key)
	{
		if (idx >= m_pKeys.size())
			return;
		m_pKeys[idx] = key;
	}

	float curvePlaySpeed;
	bool paused;
	bool loops;
	bool doesSpeedControl;
};