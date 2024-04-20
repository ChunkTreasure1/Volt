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

		/// Function from SCIRun. Here is a summary of SCIRun's description:
		/// Returns true if the two AABB's are similar. If diff is 1.0, the two
		/// bboxes have to have about 50% overlap each for x,y,z. If diff is 0.0,
		/// they have to have 100% overlap.
		/// If either of the two AABBs is NULL, then false is returned.
		/// \xxx Untested -- This function is not represented in our unit tests.
		bool IsSimilarTo(const AABB& b, float diff = 0.5) const;

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
