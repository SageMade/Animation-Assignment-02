/*
	Authors:
	Shawn M.          100412327
	Paul Puig         100656910
	Stephen Richards  100458273
*/


#pragma once

#include <vector>
#include <functional>

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
struct SpeedControlTableRowAdaptive
{
	float u0;
	float u1;
	float uMid;
	T P0;
	T Pmid;
	T P1;
	float dist1;
	float dist2;
	float dist3;
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

	void CalculateLookupTableAdaptiveSampling(float tolerance)
	{
		if (myKeyFrames.size()>3)
		{
			float uStart = 0.0f;
			float uEnd = 1.0f;
			float uMid = (uStart + uEnd) * 0.5;
			float sample;
			lookupTable.clear();

			for (int i = 0; i < myKeyFrames.size() - 3; i++)
			{
				while (uStart != 0.750f)
				{
					SpeedControlTableRowAdaptive<T> row;

					T P0 = AnimationMath::bezier(myKeyFrames[i], myKeyFrames[i + 1],
						myKeyFrames[i + 2], myKeyFrames[i + 3], uStart);
					T P1 = AnimationMath::bezier(myKeyFrames[i], myKeyFrames[i + 1],
						myKeyFrames[i + 2], myKeyFrames[i + 3], uMid);
					T P2 = AnimationMath::bezier(myKeyFrames[i], myKeyFrames[i + 1],
						myKeyFrames[i + 2], myKeyFrames[i + 3], uEnd);
					float distance1 = glm::distance(P1, P0);
					float distance2 = glm::distance(P1, P2);
					float distance3 = glm::distance(P0, P2);

					row.u0 = uStart;
					row.u1 = uEnd;
					row.uMid = uMid;
					row.P0 = P0;
					row.Pmid = P1;
					row.P1 = P2;
					row.dist1 = distance1;
					row.dist2 = distance2;
					row.dist3 = distance3;
					sample = distance1 + distance2 - distance3;
					if (sample >tolerance)
					{
						adaptiveLookupTable.push_back(row);
						uEnd = uMid;
						uMid = (uStart + uEnd) * 0.5;

					}
					else
					{
						uStart = uEnd;
						if (uStart == 0.5f)
						{
							uEnd = 1.0f;
						}
						else
						{
							uEnd = 1.0f * 0.5;
						}
						uMid = (uStart + uEnd)*0.5;

					}
				}

			}

		}

	}

	void calculateLookupTable(int numSamples, std::function<const T(T, T, float)> curve)
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

					row.sampleValue = curve(m_pKeys[i], m_pKeys[i + 1], j); // For this lab, we'll use lerp. But this exact algorithm works for catmull or bezier too.

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

		return speedControlledUpdate(dt);
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
			if (loops)
			{
				m_pCurrentKeyframe %= m_pKeys.size();
				m_pNextKeyframe    %= m_pKeys.size();
			}
			else if (m_pNextKeyframe >= m_pKeys.size())
			{
				m_pCurrentKeyframe = 0;
				m_pNextKeyframe = 1;
			}
		}
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
};