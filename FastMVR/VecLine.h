#pragma once
#include<tuple>
#include <glm/glm.hpp>
#include <optional>

/// <summary>
/// Gerade in vektorieller Darstellung
/// </summary>
class VecLine
{
public:
	// Creates a VecLine based on two points
	static VecLine create_from_positions(glm::vec3 p1, glm::vec3 p2)
	{
		return VecLine{p1, p2 - p1};
	}
	
	// Costructor
	VecLine(glm::vec3 o, glm::vec3 d)
		: m_origin{o}
		, m_direction{d}
	{}

	// Geradengleichung: G = O + d*R
	glm::vec3 origin() const { return m_origin; }
	void set_origin(glm::vec3 position) { m_origin = position; }
	glm::vec3 direction() { return m_direction; }
	void set_direction(glm::vec3 direction) { m_direction = direction; }

	void normalize_direction() { m_direction = glm::normalize(m_direction); }
	
	glm::vec3 get_position_vector_with_lambda(float lambda)
	{
		return m_origin + lambda * m_direction;
	}

	// Calculates the distance of a point to the line
	float distance_to_point(glm::vec3 point)
	{
		return glm::length(glm::cross(m_direction, point - m_origin)) / glm::length(m_direction);
	}

	// Calculates the shortest distance between two skew lines (this and otherLine)
	// and the endpoints of the line which connects those two lines at their shortest distance.
	std::optional<std::tuple<float, glm::vec3, glm::vec3>> distance_to_line(VecLine otherLine);
	
private:
	glm::vec3 m_origin;
	glm::vec3 m_direction;
};
