#pragma once

#include <algorithm>
#include <glm/glm.hpp>

namespace Volt
{
	/// Standalone axis aligned bounding box implemented built on top of GEM.
	class AABB
	{
	public:
		/// Builds a null AABB.
		AABB();

		/// Builds an AABB that encompasses a sphere.
		/// \param[in]  center Center of the sphere.
		/// \param[in]  radius Radius of the sphere.
		AABB(const glm::vec3& center, float radius);

		/// Builds an AABB that contains the two points.
		AABB(const glm::vec3& p1, const glm::vec3& p2);

		AABB(const AABB& aabb);
		~AABB() = default;

		/// Set the AABB as NULL (not set).
		void SetNull() { m_min = glm::vec3(1.0); m_max = glm::vec3(-1.0); }

		/// Returns true if AABB is NULL (not set).
		bool IsNull() const { return m_min.x > m_max.x || m_min.y > m_max.y || m_min.z > m_max.z; }

		/// Extend the bounding box on all sides by \p val.
		void Extend(float val);

		/// Expand the AABB to include point \p p.
		void Extend(const glm::vec3& p);

		/// Expand the AABB to include a sphere centered at \p center and of radius \p
		/// radius.
		/// \param[in]  center Center of sphere.
		/// \param[in]  radius Radius of sphere.
		void Extend(const glm::vec3& center, float radius);

		/// Expand the AABB to encompass the given \p aabb.
		void Extend(const AABB& aabb);

		/// Expand the AABB to include a disk centered at \p center, with normal \p
		/// normal, and radius \p radius.
		/// \xxx Untested -- This function is not represented in our unit tests.
		void ExtendDisk(const glm::vec3& center, const glm::vec3& normal,
						float radius);

		/// Translates AABB by vector \p v.
		void Translate(const glm::vec3& v);

		/// Scale the AABB by \p scale, centered around \p origin.
		/// \param[in]  scale  3D vector specifying scale along each axis.
		/// \param[in]  origin Origin of scaling operation. Most useful origin would
		///                    be the center of the AABB.
		void Scale(const glm::vec3& scale, const glm::vec3& origin);

		/// Retrieves the center of the AABB.
		glm::vec3 GetCenter() const;

		/// Retrieves the diagonal vector (computed as mMax - mMin).
		/// If the AABB is NULL, then a vector of all zeros is returned.
		glm::vec3 GetDiagonal() const;

		/// Retrieves the longest edge.
		/// If the AABB is NULL, then 0 is returned.
		float GetLongestEdge() const;

		/// Retrieves the shortest edge.
		/// If the AABB is NULL, then 0 is returned.
		float GetShortestEdge() const;

		/// Retrieves the AABB's minimum point.
		inline const glm::vec3& GetMin() const { return m_min; }

		/// Retrieves the AABB's maximum point.
		inline const glm::vec3& GetMax() const { return m_max; }

		/// Returns true if AABBs share a face overlap.
		/// \xxx Untested -- This function is not represented in our unit tests.
		bool Overlaps(const AABB& bb) const;

		/// Type returned from call to intersect.
		enum INTERSECTION_TYPE { INSIDE, INTERSECT, OUTSIDE };
		/// Returns one of the intersection types. If either of the aabbs are invalid,
		/// then OUTSIDE is returned.
		INTERSECTION_TYPE Intersect(const AABB& bb) const;

		bool IntersectTriangle(const glm::vec3& tv0, const glm::vec3& tv1, const glm::vec3& tv2);

		/// Function from SCIRun. Here is a summary of SCIRun's description:
		/// Returns true if the two AABB's are similar. If diff is 1.0, the two
		/// bboxes have to have about 50% overlap each for x,y,z. If diff is 0.0,
		/// they have to have 100% overlap.
		/// If either of the two AABBs is NULL, then false is returned.
		/// \xxx Untested -- This function is not represented in our unit tests.
		bool IsSimilarTo(const AABB& b, float diff = 0.5) const;

		VT_INLINE VT_NODISCARD bool Contains(const glm::vec3& p) const
		{
			return (p.x >= m_min.x && p.x <= m_max.x &&
					p.y >= m_min.y && p.y <= m_max.y &&
					p.z >= m_min.z && p.z <= m_max.z);
		}


	private:

		glm::vec3 m_min;   ///< Minimum point.
		glm::vec3 m_max;   ///< Maximum point.
	};

	inline AABB::AABB()
	{
		SetNull();
	}

	inline AABB::AABB(const glm::vec3& center, float radius)
	{
		SetNull();
		Extend(center, radius);
	}

	inline AABB::AABB(const glm::vec3& p1, const glm::vec3& p2)
	{
		SetNull();
		Extend(p1);
		Extend(p2);
	}

	inline AABB::AABB(const AABB& aabb)
	{
		SetNull();
		Extend(aabb);
	}

	inline void AABB::Extend(float val)
	{
		if (!IsNull())
		{
			m_min -= glm::vec3(val);
			m_max += glm::vec3(val);
		}
	}

	inline void AABB::Extend(const glm::vec3& p)
	{
		if (!IsNull())
		{
			m_min = glm::min(p, m_min);
			m_max = glm::max(p, m_max);
		}
		else
		{
			m_min = p;
			m_max = p;
		}
	}

	inline void AABB::Extend(const glm::vec3& p, float radius)
	{
		glm::vec3 r(radius);
		if (!IsNull())
		{
			m_min = glm::min(p - r, m_min);
			m_max = glm::max(p + r, m_max);
		}
		else
		{
			m_min = p - r;
			m_max = p + r;
		}
	}

	inline void AABB::Extend(const AABB& aabb)
	{
		if (!aabb.IsNull())
		{
			Extend(aabb.m_min);
			Extend(aabb.m_max);
		}
	}

	inline void AABB::ExtendDisk(const glm::vec3& c, const glm::vec3& n, float r)
	{
		if (glm::length(n) < 1.e-12) { Extend(c); return; }
		glm::vec3 norm = glm::normalize(n);
		float x = sqrt(1 - norm.x) * r;
		float y = sqrt(1 - norm.y) * r;
		float z = sqrt(1 - norm.z) * r;
		Extend(c + glm::vec3(x, y, z));
		Extend(c - glm::vec3(x, y, z));
	}

	inline glm::vec3 AABB::GetDiagonal() const
	{
		if (!IsNull())
			return m_max - m_min;
		else
			return glm::vec3(0);
	}

	inline float AABB::GetLongestEdge() const
	{
		glm::vec3 d = GetDiagonal();
		return std::max(d.x, std::max(d.y, d.z));
	}

	inline float AABB::GetShortestEdge() const
	{
		glm::vec3 d = GetDiagonal();
		return std::min(d.x, std::min(d.y, d.z));
	}

	inline glm::vec3 AABB::GetCenter() const
	{
		if (!IsNull())
		{
			glm::vec3 d = GetDiagonal();
			return m_min + (d * float(0.5f));
		}
		else
		{
			return glm::vec3(0.0);
		}
	}

	inline void AABB::Translate(const glm::vec3& v)
	{
		if (!IsNull())
		{
			m_min += v;
			m_max += v;
		}
	}

	inline void AABB::Scale(const glm::vec3& s, const glm::vec3& o)
	{
		if (!IsNull())
		{
			m_min -= o;
			m_max -= o;

			m_min *= s;
			m_max *= s;

			m_min += o;
			m_max += o;
		}
	}

	inline bool AABB::Overlaps(const AABB& bb) const
	{
		if (IsNull() || bb.IsNull())
			return false;

		if (bb.m_min.x > m_max.x || bb.m_max.x < m_min.x)
			return false;
		else if (bb.m_min.y > m_max.y || bb.m_max.y < m_min.y)
			return false;
		else if (bb.m_min.z > m_max.z || bb.m_max.z < m_min.z)
			return false;

		return true;
	}

	inline AABB::INTERSECTION_TYPE AABB::Intersect(const AABB& b) const
	{
		if (IsNull() || b.IsNull())
			return OUTSIDE;

		if ((m_max.x < b.m_min.x) || (m_min.x > b.m_max.x) ||
			(m_max.y < b.m_min.y) || (m_min.y > b.m_max.y) ||
			(m_max.z < b.m_min.z) || (m_min.z > b.m_max.z))
		{
			return OUTSIDE;
		}

		if ((m_min.x <= b.m_min.x) && (m_max.x >= b.m_max.x) &&
			(m_min.y <= b.m_min.y) && (m_max.y >= b.m_max.y) &&
			(m_min.z <= b.m_min.z) && (m_max.z >= b.m_max.z))
		{
			return INSIDE;
		}

		return INTERSECT;
	}

	// From https://gist.github.com/jflipts/fc68d4eeacfcc04fbdb2bf38e0911850
	inline bool AABB::IntersectTriangle(const glm::vec3& tv0, const glm::vec3& tv1, const glm::vec3& tv2)
	{
		glm::vec3 boxCenter = (m_min + m_max) / 2.f;
		glm::vec3 boxHalfsize = (m_max - m_min) / 2.f;

		auto findMinMax = [](float x0, float x1, float x2, float& min, float& max)
		{
			min = max = x0;
			if (x1 < min)
				min = x1;
			if (x1 > max)
				max = x1;
			if (x2 < min)
				min = x2;
			if (x2 > max)
				max = x2;
		};

		auto planeBoxOverlap = [](glm::vec3 normal, glm::vec3 vert, glm::vec3 maxbox) -> bool
		{
			glm::vec3 vmin, vmax;
			float v;
			for (uint32_t q = 0; q < 3; q++)
			{
				v = vert[q];
				if (normal[q] > 0.0f)
				{
					vmin[q] = -maxbox[q] - v;
					vmax[q] = maxbox[q] - v;
				}
				else
				{
					vmin[q] = maxbox[q] - v;
					vmax[q] = -maxbox[q] - v;
				}
			}
			if (glm::dot(normal, vmin) > 0.0f)
				return false;
			if (glm::dot(normal, vmax) >= 0.0f)
				return true;

			return false;
		};

		/*======================== X-tests ========================*/

		auto axisTestX01 = [](float a, float b, float fa, float fb, const glm::vec3& v0,
			const glm::vec3& v2, const glm::vec3& boxhalfsize, float& rad, float& min,
			float& max, float& p0, float& p2)
		{
			p0 = a * v0.y - b * v0.z;
			p2 = a * v2.y - b * v2.z;
			if (p0 < p2)
			{
				min = p0;
				max = p2;
			}
			else
			{
				min = p2;
				max = p0;
			}
			rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		auto axisTestX2 = [](float a, float b, float fa, float fb, const glm::vec3& v0,
			const glm::vec3& v1, const glm::vec3& boxhalfsize, float& rad, float& min,
			float& max, float& p0, float& p1)
		{
			p0 = a * v0.y - b * v0.z;
			p1 = a * v1.y - b * v1.z;
			if (p0 < p1)
			{
				min = p0;
				max = p1;
			}
			else
			{
				min = p1;
				max = p0;
			}
			rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		/*======================== Y-tests ========================*/

		auto axisTestY02 = [](float a, float b, float fa, float fb, const glm::vec3& v0,
			const glm::vec3& v2, const glm::vec3& boxhalfsize, float& rad, float& min,
			float& max, float& p0, float& p2)
		{
			p0 = -a * v0.x + b * v0.z;
			p2 = -a * v2.x + b * v2.z;
			if (p0 < p2)
			{
				min = p0;
				max = p2;
			}
			else
			{
				min = p2;
				max = p0;
			}
			rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		auto axisTestY1 = [](float a, float b, float fa, float fb, const glm::vec3& v0,
			const glm::vec3& v1, const glm::vec3& boxhalfsize, float& rad, float& min,
			float& max, float& p0, float& p1)
		{
			p0 = -a * v0.x + b * v0.z;
			p1 = -a * v1.x + b * v1.z;
			if (p0 < p1)
			{
				min = p0;
				max = p1;
			}
			else
			{
				min = p1;
				max = p0;
			}
			rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		/*======================== Z-tests ========================*/
		auto axisTestZ12 = [](float a, float b, float fa, float fb, const glm::vec3 & v1,
			const glm::vec3 & v2, const glm::vec3 & boxhalfsize, float& rad, float& min,
			float& max, float& p1, float& p2)
		{
			p1 = a * v1.x - b * v1.y;
			p2 = a * v2.x - b * v2.y;
			if (p1 < p2)
			{
				min = p1;
				max = p2;
			}
			else
			{
				min = p2;
				max = p1;
			}
			rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		auto axisTestZ0 = [](float a, float b, float fa, float fb, const glm::vec3& v0,
			const glm::vec3& v1, const glm::vec3& boxhalfsize, float& rad, float& min,
			float& max, float& p0, float& p1)
		{
			p0 = a * v0.x - b * v0.y;
			p1 = a * v1.x - b * v1.y;
			if (p0 < p1)
			{
				min = p0;
				max = p1;
			}
			else
			{
				min = p1;
				max = p0;
			}
			rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
			if (min > rad || max < -rad)
				return false;
			return true;
		};

		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */
		glm::vec3 v0, v1, v2;
		float min, max, p0, p1, p2, rad, fex, fey, fez;
		glm::vec3 normal, e0, e1, e2;

		/* This is the fastest branch on Sun */
		/* move everything so that the boxcenter is in (0,0,0) */
		v0 = tv0 - boxCenter;
		v1 = tv1 - boxCenter;
		v2 = tv2 - boxCenter;

		/* compute triangle edges */
		e0 = v1 - v0;
		e1 = v2 - v1;
		e2 = v0 - v2;

		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */
		fex = fabsf(e0.x);
		fey = fabsf(e0.y);
		fez = fabsf(e0.z);

		if (!axisTestX01(e0.z, e0.y, fez, fey, v0, v2, boxHalfsize, rad, min, max, p0, p2))
			return false;
		if (!axisTestY02(e0.z, e0.x, fez, fex, v0, v2, boxHalfsize, rad, min, max, p0, p2))
			return false;
		if (!axisTestZ12(e0.y, e0.x, fey, fex, v1, v2, boxHalfsize, rad, min, max, p1, p2))
			return false;

		fex = fabsf(e1.x);
		fey = fabsf(e1.y);
		fez = fabsf(e1.z);

		if (!axisTestX01(e1.z, e1.y, fez, fey, v0, v2, boxHalfsize, rad, min, max, p0, p2))
			return false;
		if (!axisTestY02(e1.z, e1.x, fez, fex, v0, v2, boxHalfsize, rad, min, max, p0, p2))
			return false;
		if (!axisTestZ0(e1.y, e1.x, fey, fex, v0, v1, boxHalfsize, rad, min, max, p0, p1))
			return false;

		fex = fabsf(e2.x);
		fey = fabsf(e2.y);
		fez = fabsf(e2.z);
		if (!axisTestX2(e2.z, e2.y, fez, fey, v0, v1, boxHalfsize, rad, min, max, p0, p1))
			return false;
		if (!axisTestY1(e2.z, e2.x, fez, fex, v0, v1, boxHalfsize, rad, min, max, p0, p1))
			return false;
		if (!axisTestZ12(e2.y, e2.x, fey, fex, v1, v2, boxHalfsize, rad, min, max, p1, p2))
			return false;

		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */

		/* test in X-direction */
		findMinMax(v0.x, v1.x, v2.x, min, max);
		if (min > boxHalfsize.x || max < -boxHalfsize.x)
			return false;

		/* test in Y-direction */
		findMinMax(v0.y, v1.y, v2.y, min, max);
		if (min > boxHalfsize.y || max < -boxHalfsize.y)
			return false;

		/* test in Z-direction */
		findMinMax(v0.z, v1.z, v2.z, min, max);
		if (min > boxHalfsize.z || max < -boxHalfsize.z)
			return false;

		/* Bullet 2: */
		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */
		normal = glm::cross(e0, e1);
		if (!planeBoxOverlap(normal, v0, boxHalfsize))
			return false;

		return true; /* box and triangle overlaps */
	}

	inline bool AABB::IsSimilarTo(const AABB& b, float diff) const
	{
		if (IsNull() || b.IsNull()) return false;

		glm::vec3 acceptable_diff = ((GetDiagonal() + b.GetDiagonal()) / float(2.0f)) * diff;
		glm::vec3 min_diff(m_min - b.m_min);
		min_diff = glm::vec3(fabs(min_diff.x), fabs(min_diff.y), fabs(min_diff.z));
		if (min_diff.x > acceptable_diff.x) return false;
		if (min_diff.y > acceptable_diff.y) return false;
		if (min_diff.z > acceptable_diff.z) return false;
		glm::vec3 max_diff(m_max - b.m_max);
		max_diff = glm::vec3(fabs(max_diff.x), fabs(max_diff.y), fabs(max_diff.z));
		if (max_diff.x > acceptable_diff.x) return false;
		if (max_diff.y > acceptable_diff.y) return false;
		if (max_diff.z > acceptable_diff.z) return false;
		return true;
	}
}
