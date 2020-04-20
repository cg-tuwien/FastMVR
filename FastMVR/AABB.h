#pragma once
#include <glm/vec3.hpp>
#include <vector>
#include <limits>
#define GL_SCENE
#include "Scene.h"
#undef GL_SCENE

namespace mymath
{
	enum struct location_wrt_aabb
	{
		inside,
		outside,
		on_the_edge
	};

	class AABB
	{
		friend bool operator==(const AABB& left, const AABB& right);
	public:
		glm::vec3 min_coords() const { return glm::vec3(min_x, min_y, min_z); }
		glm::vec3 max_coords() const { return glm::vec3(max_x, max_y, max_z); }
		glm::vec3 extent() const { return max_coords() - min_coords(); }

		location_wrt_aabb classify_point(glm::vec3 pt) const
		{
			auto diff1 = pt - min_coords();
			auto diff2 = max_coords() - pt;
			if (diff1.x > 0 && diff1.y > 0 && diff1.z > 0 && diff2.x > 0 && diff2.y > 0 && diff2.z > 0) {
				return location_wrt_aabb::inside;
			}
			if (diff1.x >= 0 && diff1.y >= 0 && diff1.z >= 0 && diff2.x >= 0 && diff2.y >= 0 && diff2.z >= 0) {
				return location_wrt_aabb::on_the_edge;
			}
			return location_wrt_aabb::outside;
		}

		//location_wrt_aabb classify_triangle

		static AABB create_for_vertices(const std::vector<SceneVertex>& vertices)
		{
			assert(vertices.size() > 0);
			AABB result;
			result.min_x = std::numeric_limits<float>::max();
			result.min_y = std::numeric_limits<float>::max();
			result.min_z = std::numeric_limits<float>::max();
			result.max_x = std::numeric_limits<float>::min();
			result.max_y = std::numeric_limits<float>::min();
			result.max_z = std::numeric_limits<float>::min();
			for (const auto& sv : vertices) {
				result.min_x = glm::min(result.min_x, sv.position.x);
				result.min_y = glm::min(result.min_y, sv.position.y);
				result.min_z = glm::min(result.min_z, sv.position.z);
				result.max_x = glm::max(result.max_x, sv.position.x);
				result.max_y = glm::max(result.max_y, sv.position.y);
				result.max_z = glm::max(result.max_z, sv.position.z);
			}
			return result;
		}

		static AABB create_for_min_max_coords(glm::vec3 minCoords, glm::vec3 maxCoords)
		{
			AABB result;
			result.min_x = minCoords.x;
			result.min_y = minCoords.y;
			result.min_z = minCoords.z;
			result.max_x = maxCoords.x;
			result.max_y = maxCoords.y;
			result.max_z = maxCoords.z;
			return result;
		}

	private:
		float min_x;
		float min_y;
		float min_z;
		float max_x;
		float max_y;
		float max_z;
	};

	static bool operator==(const AABB& left, const AABB& right)
	{
		if (std::abs(left.min_x - right.min_x) > std::numeric_limits<float>::epsilon()) return false;
		if (std::abs(left.min_y - right.min_y) > std::numeric_limits<float>::epsilon()) return false;
		if (std::abs(left.min_z - right.min_z) > std::numeric_limits<float>::epsilon()) return false;
		if (std::abs(left.min_x - right.min_x) > std::numeric_limits<float>::epsilon()) return false;
		if (std::abs(left.min_y - right.min_y) > std::numeric_limits<float>::epsilon()) return false;
		if (std::abs(left.min_z - right.min_z) > std::numeric_limits<float>::epsilon()) return false;
		return true;
	}

}