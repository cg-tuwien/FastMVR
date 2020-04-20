#include "VecPlane.h"
#include "VecLine.h"

// Calculates the intersection line between two planes.
// Returns the intersection line if the two planes intersect, std::nullopt otherwise.
std::optional<VecLine> VecPlane::get_intersection_line(VecPlane first, VecPlane second)
{
	// The direction vector of the lines is the cross product of both planes' normal vectors
	glm::vec3 r = glm::cross(first.normal(), second.normal());

	// The origin must be found by solving a linear system of equations:
	auto n1 = first.normal();
	auto n2 = second.normal();
	auto A = glm::mat2{{n1.y, n2.z}, {n2.y, n2.z}};
	auto o1 = first.origin();
	auto o2 = second.origin();
	auto b = glm::vec2{glm::dot(n1, o1), glm::dot(n2, o2)};
	auto x = solve_system_of_equations(A, b);
	return x.has_value()
		? VecLine(glm::vec3(0.0f, x.value().x, x.value().y), r)
		: std::optional<VecLine>{};
}

std::optional<glm::vec3> VecPlane::get_intersection_point(VecPlane plane, VecLine line)
{
	// Calculate lambda:
	float n_dot_d = glm::dot(plane.normal(), line.direction());
	if (glm::abs(n_dot_d) < std::numeric_limits<float>::epsilon()) {
		return {};
	}
	auto lambda = glm::dot(plane.normal(), plane.origin() - line.origin()) / n_dot_d;
	return line.get_position_vector_with_lambda(lambda);
}
