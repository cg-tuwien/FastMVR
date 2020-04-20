#pragma once

#include "GeometryProvider.h"
#include <MathGeoLib.h>
#include <stack>

#pragma region Geometry Structs and Frustum Plane extraction
// Credits: Gil Gribb and Klaus Hartmann, Fast Extraction of Viewing Frustum Planes from the WorldView-Projection Matrix
// online; accessed 09.10.2019 https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf

namespace gamedevs
{

	struct Point
	{
	 float x, y, z;
	};

	static Point ConvertFromGlm(glm::vec3 gv)
	{
		return Point{ gv.x, gv.y, gv.z };
	}

	struct Plane
	{
	 float a, b, c, d;
	};

	static void NormalizePlane(Plane & plane)
	{
	 float mag;
	 mag = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
	 plane.a = plane.a / mag;
	 plane.b = plane.b / mag;
	 plane.c = plane.c / mag;
	 plane.d = plane.d / mag;
	}

	static float DistanceToPoint(const Plane & plane, const Point & pt)
	{
	 return plane.a*pt.x + plane.b*pt.y + plane.c*pt.z + plane.d;
	}

	enum struct Halfspace
	{
	 NEGATIVE = -1,
	 ON_PLANE = 0,
	 POSITIVE = 1,
	};

	static Halfspace ClassifyPoint(const Plane & plane, const Point & pt)
	{
	 float d;
	 d = plane.a*pt.x + plane.b*pt.y + plane.c*pt.z + plane.d;
	 if (d < 0) return Halfspace::NEGATIVE;
	 if (d > 0) return Halfspace::POSITIVE;
	 return Halfspace::ON_PLANE;
	}

	static Halfspace ClassifyPoint(const Plane& plane, glm::vec3 point)
	{
		Point pointConv = ConvertFromGlm(point);
		return ClassifyPoint(plane, pointConv);
	}

	struct Matrix4x4
	{
	 // The elements of the 4x4 matrix are stored in
	 // column-major order (see "OpenGL Programming Guide",
	 // 3rd edition, pp 106, glLoadMatrix).
	 float _11, _21, _31, _41;
	 float _12, _22, _32, _42;
	 float _13, _23, _33, _43;
	 float _14, _24, _34, _44;
	};

	static Matrix4x4 ConvertFromGlm(glm::mat4 gm)
	{
		Matrix4x4 result;
		memcpy(&result, glm::value_ptr(gm), sizeof(Matrix4x4));
		return result;
	}

	// Extracts frustum planes from a matrix in the following order: left, right, top, bottom, near, far
	static void ExtractPlanesGL(
	 Plane * p_planes,
	 const Matrix4x4 & comboMatrix,
	 bool normalize)
	{
	 // Left clipping plane
	 p_planes[0].a = comboMatrix._41 + comboMatrix._11;
	 p_planes[0].b = comboMatrix._42 + comboMatrix._12;
	 p_planes[0].c = comboMatrix._43 + comboMatrix._13;
	 p_planes[0].d = comboMatrix._44 + comboMatrix._14;
	 // Right clipping plane
	 p_planes[1].a = comboMatrix._41 - comboMatrix._11;
	 p_planes[1].b = comboMatrix._42 - comboMatrix._12;
	 p_planes[1].c = comboMatrix._43 - comboMatrix._13;
	 p_planes[1].d = comboMatrix._44 - comboMatrix._14;
	 // Top clipping plane
	 p_planes[2].a = comboMatrix._41 - comboMatrix._21;
	 p_planes[2].b = comboMatrix._42 - comboMatrix._22;
	 p_planes[2].c = comboMatrix._43 - comboMatrix._23;
	 p_planes[2].d = comboMatrix._44 - comboMatrix._24;
	 // Bottom clipping plane
	 p_planes[3].a = comboMatrix._41 + comboMatrix._21;
	 p_planes[3].b = comboMatrix._42 + comboMatrix._22;
	 p_planes[3].c = comboMatrix._43 + comboMatrix._23;
	 p_planes[3].d = comboMatrix._44 + comboMatrix._24;
	 // Near clipping plane
	 p_planes[4].a = comboMatrix._41 + comboMatrix._31;
	 p_planes[4].b = comboMatrix._42 + comboMatrix._32;
	 p_planes[4].c = comboMatrix._43 + comboMatrix._33;
	 p_planes[4].d = comboMatrix._44 + comboMatrix._34;
	 // Far clipping plane
	 p_planes[5].a = comboMatrix._41 - comboMatrix._31;
	 p_planes[5].b = comboMatrix._42 - comboMatrix._32;
	 p_planes[5].c = comboMatrix._43 - comboMatrix._33;
	 p_planes[5].d = comboMatrix._44 - comboMatrix._34;
	 // Normalize the plane equations, if requested
	 if (normalize == true)
	 {
	 NormalizePlane(p_planes[0]);
	 NormalizePlane(p_planes[1]);
	 NormalizePlane(p_planes[2]);
	 NormalizePlane(p_planes[3]);
	 NormalizePlane(p_planes[4]);
	 NormalizePlane(p_planes[5]);
	 }
	}

	// Extracts frustum planes from a matrix in the following order: left, right, top, bottom, near, far
	static std::array<Plane, 6> ExtractPlanesGL(glm::mat4 comboMatrix, bool normalize)
	{
		std::array<Plane, 6> result;
		Matrix4x4 matConv = ConvertFromGlm(comboMatrix);
		ExtractPlanesGL(result.data(), matConv, normalize);
		return result;
	}

	// If ALL points are outside the frustum, returns false
	// If ANY point is inside the frustum, returns true
	static bool IsAnyPointInsideTheFrustum(std::vector<glm::vec3> points, std::array<Plane, 6> frustum)
	{
		std::vector<bool> outsideOfAnyPlane;
		for (size_t i = 0; i < points.size(); ++i) {
			outsideOfAnyPlane.push_back(false);
		}
		
		for (auto& plane : frustum) {
			for (size_t i = 0; i < points.size(); ++i) {
				outsideOfAnyPlane[i] = outsideOfAnyPlane[i] || (ClassifyPoint(plane, points[i]) == Halfspace::NEGATIVE);
			}
		}

		for (size_t i = 0; i < points.size(); ++i) {
			if (!outsideOfAnyPlane[i]) {
				return true;
			}
		}
		return false;
	}

}
	
#pragma endregion 

static float3 convert_to_float3(glm::vec3 input)
{
	return float3{input.x, input.y, input.z};
}

static glm::vec3 convert_to_vec3(float3 input)
{
	return glm::vec3{input.x, input.y, input.z};
}

static math::Plane convert_to_math_plane(gamedevs::Plane input)
{
	// Reverse the normal
	auto normalVector = -float3{input.a, input.b, input.c};
	// Keep the distance
	auto distance = input.d;
	// That's how you convert a gamedev::Plane into a math::Plane
	return math::Plane(normalVector, distance);
}

static math::Frustum convert_to_frustum(math::AABB aabb)
{
	math::Frustum frust;
	frust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	frust.SetViewPlaneDistances(0, aabb.Size().z);
	frust.SetFrame(float3{
		aabb.MinX() + aabb.HalfSize().x,
		aabb.MinY() + aabb.HalfSize().y,
		aabb.MaxZ()}, float3{0.f, 0.f, -1.f}, float3{0.f, 1.f, 0.f});
	frust.SetOrthographic(aabb.Size().x, aabb.Size().y);
	return frust;
}

static std::array<math::Plane, 6> convert_to_math_frustum_planes(std::array<gamedevs::Plane, 6> input)
{
	std::array<math::Plane, 6> result;
	for (size_t i = 0; i < 6; ++i) {
		result[i] = convert_to_math_plane(input[i]);
	}
	return result;
}

static void NormalizePlane(math::Plane& plane)
{
	float mag;
	mag = sqrt(plane.normal.x * plane.normal.x + plane.normal.y * plane.normal.y + plane.normal.z * plane.normal.z);
	plane.normal.x = plane.normal.x / mag;
	plane.normal.y = plane.normal.y / mag;
	plane.normal.z = plane.normal.z / mag;
	plane.d = plane.d / mag;
}


static std::array<math::Plane, 6> get_planes_of_aabb(math::AABB input, bool normalize = false)
{
	std::array<math::Plane, 6> result;
	// Left clipping plane
	result[0] = math::Plane{
		float3{input.minPoint.x, input.minPoint.y, input.maxPoint.z},
		float3{input.minPoint.x, input.maxPoint.y, input.maxPoint.z},
		float3{input.minPoint.x, input.minPoint.y, input.minPoint.z}
	};
	// Right clipping plane
	result[1] = math::Plane{
		float3{input.maxPoint.x, input.minPoint.y, input.minPoint.z},
		float3{input.maxPoint.x, input.maxPoint.y, input.minPoint.z},
		float3{input.maxPoint.x, input.minPoint.y, input.maxPoint.z}
	};
	// Top clipping plane
	result[2] = math::Plane{
		float3{input.minPoint.x, input.maxPoint.y, input.minPoint.z},
		float3{input.minPoint.x, input.maxPoint.y, input.maxPoint.z},
		float3{input.maxPoint.x, input.maxPoint.y, input.minPoint.z}
	};
	// Bottom clipping plane
	result[3] = math::Plane{
		float3{input.minPoint.x, input.minPoint.y, input.maxPoint.z},
		float3{input.minPoint.x, input.minPoint.y, input.minPoint.z},
		float3{input.maxPoint.x, input.minPoint.y, input.maxPoint.z}
	};
	// Near clipping plane
	result[4] = math::Plane{
		float3{input.minPoint.x, input.minPoint.y, input.maxPoint.z},
		float3{input.maxPoint.x, input.minPoint.y, input.maxPoint.z},
		float3{input.minPoint.x, input.maxPoint.y, input.maxPoint.z}
	};
	// Far clipping plane
	result[5] = math::Plane{
		float3{input.maxPoint.x, input.minPoint.y, input.minPoint.z},
		float3{input.minPoint.x, input.minPoint.y, input.minPoint.z},
		float3{input.maxPoint.x, input.maxPoint.y, input.minPoint.z}
	};
	// Normalize the plane equations, if requested
	if (normalize == true)
	{
		NormalizePlane(result[0]);
		NormalizePlane(result[1]);
		NormalizePlane(result[2]);
		NormalizePlane(result[3]);
		NormalizePlane(result[4]);
		NormalizePlane(result[5]);
	}
	return result;
}

// Triangle faces pointing outwards
static bool is_triangle_in_frustum(std::array<math::Plane, 6> frustumPlanes, math::Triangle triangle)
{
	// See if we can get away by easily testing all the points against all the frustum planes:
	std::array<int, 3> insideAllPlanesCount = {0, 0, 0};
	for (size_t i = 0; i < 6; ++i) {
		insideAllPlanesCount[0] += (frustumPlanes[i].IsOnPositiveSide(triangle.Vertex(0)) ? 0 : 1);
		insideAllPlanesCount[1] += (frustumPlanes[i].IsOnPositiveSide(triangle.Vertex(1)) ? 0 : 1);
		insideAllPlanesCount[2] += (frustumPlanes[i].IsOnPositiveSide(triangle.Vertex(2)) ? 0 : 1);
	}
	// If at least one point was inside of all the planes, we're done
	if (6 == insideAllPlanesCount[0] || 6 == insideAllPlanesCount[1] || 6 == insideAllPlanesCount[2]) {
		return true;
	}
	// We weren't lucky with our previous tests => test all the lines of the triangle against the frustum planes
	for (size_t t = 0; t < 3; ++t) {
		LineSegment ls{triangle.Vertex(t), triangle.Vertex((t + 1) % 3)};
		for (size_t i = 0; i < 6; ++i) {
			// IF they intersect, we need to check the intersection point w.r.t. all the other planes.
			float d;
			if (ls.Intersects(frustumPlanes[i], &d)) {
				auto intersectionPoint = ls.GetPoint(d);
				std::array<int, 6> info;
				int insideOtherPlanesCount = 0;
				for (size_t j = 0; j < 6; ++j) {
					if (i == j) { continue; }
					info[j] = (frustumPlanes[j].IsOnPositiveSide(intersectionPoint) ? 0 : 1);
					insideOtherPlanesCount += info[j];
				}
				// Check if the intersection point is inside the frustum
				if (5 == insideOtherPlanesCount) {
					return true;
				}
				// All checks have passed (without returning true) => proceed with next triangle line.
			}
		}		
	}
	// No intersection found which is inside the frustum => triangle must be outside.
	return false;
}

static bool is_aabb_in_frustum(std::array<math::Plane, 6> frustumPlanes, math::AABB aabb)
{
	std::array corners = {
		float3{aabb.MinX(), aabb.MinY(), aabb.MinZ()},
		float3{aabb.MaxX(), aabb.MinY(), aabb.MinZ()},
		float3{aabb.MaxX(), aabb.MaxY(), aabb.MinZ()},
		float3{aabb.MinX(), aabb.MaxY(), aabb.MinZ()},
		float3{aabb.MinX(), aabb.MinY(), aabb.MaxZ()},
		float3{aabb.MaxX(), aabb.MinY(), aabb.MaxZ()},
		float3{aabb.MaxX(), aabb.MaxY(), aabb.MaxZ()},
		float3{aabb.MinX(), aabb.MaxY(), aabb.MaxZ()}
	};
	assert(corners.size() == 8);

	// See if we can get away by easily testing all the points against all the frustum planes:
	std::array<int, 8> insideAllPlanesCount = {0, 0, 0, 0, 0, 0, 0, 0};
	for (size_t i = 0; i < 6; ++i) {
		for (size_t c = 0; c < 8; ++c) {
			insideAllPlanesCount[c] += (frustumPlanes[i].IsOnPositiveSide(corners[c]) ? 0 : 1);
		}
	}
	// If at least one point was inside of all the planes, we're done
	for (size_t c = 0; c < 8; ++c) {
		if (6 == insideAllPlanesCount[c]) {
			return true;
		}
	}
	// We weren't lucky with our previous tests => test all the lines of the AABB against the frustum planes
	std::array lineSegments = {
		LineSegment{corners[0], corners[1]},
		LineSegment{corners[1], corners[2]},
		LineSegment{corners[2], corners[3]},
		LineSegment{corners[3], corners[0]},
		LineSegment{corners[4], corners[5]},
		LineSegment{corners[5], corners[6]},
		LineSegment{corners[6], corners[7]},
		LineSegment{corners[7], corners[4]},
		LineSegment{corners[0], corners[4]},
		LineSegment{corners[1], corners[5]},
		LineSegment{corners[2], corners[6]},
		LineSegment{corners[3], corners[7]},
	};
	assert(lineSegments.size() == 12);
	for (size_t l = 0; l < 12; ++l) {
		for (size_t i = 0; i < 6; ++i) {
			// IF they intersect, we need to check the intersection point w.r.t. all the other planes.
			float d;
			if (lineSegments[l].Intersects(frustumPlanes[i], &d)) {
				auto intersectionPoint = lineSegments[l].GetPoint(d);
				int insideOtherPlanesCount = 0;
				for (size_t j = 0; j < 6; ++j) {
					if (i == j) { continue; }
					insideOtherPlanesCount += (frustumPlanes[j].IsOnPositiveSide(intersectionPoint) ? 0 : 1);
				}
				// Check if the intersection point is inside the frustum
				if (5 == insideOtherPlanesCount) {
					return true;
				}
				// All checks have passed (without returning true) => proceed with next AABB-line.
			}
		}		
	}
	// No intersection found which is inside the frustum => AABB must be outside.
	return false;
}


// Triangle faces pointing outwards
static bool does_triangle_really_intersect_one_of_the_frustum_planes(std::array<math::Plane, 6> frustumPlanes, math::Triangle triangle)
{
	for (size_t i = 0; i < 6; ++i) {
		if (frustumPlanes[i].Intersects(triangle)) {
			return true;
		}
	}
	return false;
}

static math::Frustum convert_symmetric_perspective_matrix_to_frustum(glm::mat4 _PerspectiveProjectionMatrix)
{
	const auto& m = _PerspectiveProjectionMatrix;
	float zNear		= m[3][2] / (m[2][2] - 1.f);
	float zFar		= m[3][2] / (m[2][2] + 1.f);
	//float bottom	= zNear * (m[2][1] - 1.f) / m[1][1];
	//float top		= zNear * (m[2][1] + 1.f) / m[1][1];
	//float left		= zNear * (m[2][0] - 1.f) / m[0][0];
	//float right		= zNear * (m[2][0] + 1.f) / m[0][0];
	
	float angVert = std::atanf(1.f / m[1][1]) * 2.f;
	float aspect = m[1][1] / m[0][0];
	float angHoriz = aspect * angVert;

#if defined (_DEBUG)
	auto test = glm::perspective(angVert, aspect, zNear, zFar);
	for (size_t i=0; i < 4; ++i) {
		for (size_t j=0; i < 4; ++i) {
			assert(std::abs(_PerspectiveProjectionMatrix[i][j] - test[i][j]) <= 0.0001f);
		}
	}
#endif
	
	auto frust = math::Frustum();
	frust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	frust.SetViewPlaneDistances(zNear, zFar);
	frust.SetVerticalFovAndAspectRatio(angVert, aspect);

	return frust;
}

static std::vector<math::Triangle> clip_against_frustum_planes(math::Triangle triangle, std::array<math::Plane, 6> frustumPlanes)
{
	std::vector<math::Triangle> clipStack;
	clipStack.push_back(triangle);

	math::Triangle tmp1, tmp2;
	// Clip against all frustum planes
	for (size_t i = 0; i < 6; ++i) {
		auto plane = frustumPlanes[i];
		plane.ReverseNormal(); // The part of the triangle which lies in the negative halfspace of the plane is removed.

		auto toProcess = clipStack;
		clipStack.clear();
		for (auto& tri : toProcess) {
			auto numTris = plane.Clip(tri, tmp1, tmp2);
			if (numTris >= 1) {
				clipStack.push_back(tmp1);
				if (2 == numTris) {
					clipStack.push_back(tmp2);
				}
			}
		}
	}

	return clipStack; // contains the final triangle results, after having clipped them against all frustum planes.
}