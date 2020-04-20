#pragma once
#include <glm/glm.hpp>
#include <tuple>
#include <optional>

class VecLine;

// Solve a system of equations given in the form Ax = b, where A and b are
// provided, and x is returned IF the system of equations CAN be solved.
template <typename M, typename V>
static std::optional<V> solve_system_of_equations(M A, V b)
{
	if (glm::determinant(A) < std::numeric_limits<float>::epsilon()) {
		return {};
	}
	return glm::inverse(A) * b;
}

// Plane in vectorial form
class VecPlane
{
public:
	
	// Create a plane P in vector form according to the formula P = p1 + a*(p2-p1) + b*(p3-p1)
	// from three points, given in counter-clockwise order. 
	static VecPlane create_from_points_ccw(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
	{
		return VecPlane{p1, p2 - p1, p3 - p1};
	}

	// Create a plane P in vector form according to the formula P = p1 + a*(p2-p1) + b*(p3-p1)
	// from three points, given in counter-clockwise order. Normalizes the direction vectors.
	static VecPlane create_from_points_ccw_normalized(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
	{
		return VecPlane{p1, glm::normalize(p2 - p1), glm::normalize(p3 - p1)};
	}

	//static VecPlane create_from_cartesian_form(float a, float b, float c, float d)
	//{
	//	auto x1 = a/d;
	//	return VecPlane{
	//		glm::vec3{ x1, 0.f, 0.f},
	//		glm::vec3{-x1, b/d, 0.f},
	//		glm::vec3{-x1, 0.f, c/d}
	//	};
	//}

	//static VecPlane create_from_cartesian_form_normalized(float a, float b, float c, float d)
	//{
	//	auto x1 = a/d;
	//	auto nonnorm = VecPlane{
	//		glm::vec3{ x1, 0.f, 0.f},
	//		glm::vec3{-x1, b/d, 0.f},
	//		glm::vec3{-x1, 0.f, c/d}
	//	};
	//	auto yesnorm = VecPlane{
	//		glm::vec3{ x1, 0.f, 0.f},
	//		glm::normalize(glm::vec3{-x1, b/d, 0.f}),
	//		glm::normalize(glm::vec3{-x1, 0.f, c/d})
	//	};
	//	return nonnorm;
	//}

	//static VecPlane create_from_cartesian_form(glm::vec4 coefficients)
	//{
	//	return create_from_cartesian_form(coefficients.x, coefficients.y, coefficients.z, coefficients.w);
	//}

	//static VecPlane create_from_cartesian_form_normalized(glm::vec4 coefficients)
	//{
	//	return create_from_cartesian_form_normalized(coefficients.x, coefficients.y, coefficients.z, coefficients.w);
	//}

	// Construct a VecPlane
	VecPlane(glm::vec3 origin, glm::vec3 direction1, glm::vec3 direction2)
		: m_origin{origin}
		, m_direction1{direction1}
		, m_direction2{direction2}
	{ }

	// Get the origin
	glm::vec3 origin() const { return m_origin; }
	// Set new origin
	void set_origin(glm::vec3 position) { m_origin = position; }
	// Get the direction vectors
	std::tuple<glm::vec3, glm::vec3> directions() const { return std::make_tuple(m_direction1, m_direction2); }
	// Set new direction vectors
	void set_directions(glm::vec3 direction1, glm::vec3 direction2) { m_direction1 = direction1; m_direction2 = direction2; }

	// Get the normal
	glm::vec3 normal() const { return glm::cross(m_direction1, m_direction2); }
	// Get the normalized normal
	glm::vec3 normalized_normal() const { return glm::normalize(normal()); }

	// Get the plane equation in cartesian form
	glm::vec4 cartesian_form() const
	{
		auto n = normal();
		return glm::vec4{n, glm::dot(m_origin, n)};
	}

	// Solves the plane equation (using `cartesian_form()`) for the given
	// position and returns the result.
	float solve_plane_equation_for_position(glm::vec3 position)
	{
		return glm::dot(cartesian_form(), glm::vec4{position, 1});
	}

	// Gets a point on the plane which is defined by the offsets a, and b
	glm::vec3 get_position_vector_from_offsets(float a, float b)
	{
		return m_origin + a * m_direction1 + b * m_direction2;
	}

	// Calculates the intersection line between two planes.
	// Returns the intersection line if the two planes intersect, std::nullopt otherwise.
	static std::optional<VecLine> get_intersection_line(VecPlane first, VecPlane second);

	// Calculates the intersection point between a plane and a line.
	// Returns the position where the line intersects the plane if they intersect, std::nullopt otherwise.
	static std::optional<glm::vec3> get_intersection_point(VecPlane plane, VecLine line);
	
private:
	glm::vec3 m_origin;
	glm::vec3 m_direction1, m_direction2;

};
