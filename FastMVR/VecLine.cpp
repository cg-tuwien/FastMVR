#include "VecLine.h"
#include "VecPlane.h"

std::optional<std::tuple<float, glm::vec3, glm::vec3>> VecLine::distance_to_line(VecLine otherLine)
{
	// see: http://www.physicsforums.com/archive/index.php/t-13858.html
	// 1. Normalize-Cross-Prod the directions of L1 and L2 to obtain the direction of their adjoining lineSeg dL
	// 2. Extrude L1 along direction dL (= plane PL1)
	// 3. Extrude L2 along direction dL (= plane PL2)
	// 4. The endpoints of the shortest LineSeg bridging between L1 and L2 are:
	// intersection( PL1 , L2 )
	// intersection( PL2 , L1 )
	glm::vec3 dL = glm::cross(direction(), otherLine.direction());
	dL = glm::normalize(dL);

	auto pl1 = VecPlane::create_from_points_ccw(origin(), direction(), dL);
	auto pl2 = VecPlane::create_from_points_ccw(otherLine.origin(), otherLine.direction(), dL);
	auto endpoint1 = VecPlane::get_intersection_point(pl1, otherLine);
	auto endpoint2 = VecPlane::get_intersection_point(pl2, *this);
	if (endpoint1.has_value() && endpoint2.has_value()) {
		return std::make_tuple(glm::distance(endpoint1.value(), endpoint2.value()), endpoint1.value(), endpoint2.value());
	}
	return {};
}