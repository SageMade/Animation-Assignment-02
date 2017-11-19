#pragma once
#include <vector>
#include <functional>
#include "AnimationMath.h"
#include "GLM.h"
#include <TTK\GraphicsUtils.h>

template <typename T>
class AdaptiveCurve {
	public:
		AdaptiveCurve() { }
		AdaptiveCurve(std::function<const T&(float)> curve) : myCurveSolve(curve) {}
		void Bake(float tolerance = 0.1f) {
			__Generate(tolerance);
		}
		T Solve(float t) {
			if (myEntries.size() == 0 )
				return T();
			if (t < 0)
				return myEntries[0].Value;

			CurveEntry* ptr = myEntries.data() + 1;
			CurveEntry* last = ptr + myEntries.size();

			if (t > 1)
				return last->Value;

			for (; ptr <= last; ptr++) {
				if (ptr->Time >= t) {
					CurveEntry* prev = ptr - 1;
					float tCalc = (t - prev->Time) / (ptr->Time - prev->Time);
					return Math::lerp(prev->Value, ptr->Value, tCalc);
				}
			}
		}
		int SampleCount() const { return myEntries.size(); }
		T SampleAtIndex(int index) const { return myEntries[index].Value; }

	private:
		struct CurveEntry {
			T     Value;
			float Time;

			CurveEntry(const T& value, float time) : Value(value), Time(time) {
			}
		};

		void __Generate(float tolerance = 0.1f) {
			myEntries.push_back(CurveEntry(myCurveSolve(0.0f), 0.0f));
			myEntries.push_back(CurveEntry(myCurveSolve(1.0f), 1.0f));
			int ix0 = 0;
			__Resolve(ix0, 0.0f, 1.0f, myEntries[0].Value, myEntries[1].Value, tolerance);
		}

		void __Resolve(int& t0Index, float t0, float t1, T p1, T p2, float tolerance = 0.1f) {
			float tMid = (t0 + t1) / 2.0f;
			T pMid = myCurveSolve(tMid);

			float dist1 = glm::distance(p1, pMid);
			float dist2 = glm::distance(pMid, p2);
			float dist3 = glm::distance(p1, p2);

			float error = dist1 + dist2 - dist3;

			if (error > tolerance) {
				myEntries.insert(myEntries.begin() + (t0Index + 1), CurveEntry(pMid, tMid));
				__Resolve(t0Index, t0, tMid, p1, pMid);
				t0Index++;
				__Resolve(t0Index, tMid, t1, pMid, p2);
			}
		}

		std::vector<CurveEntry> myEntries;
		std::function<T(float)> myCurveSolve;
};