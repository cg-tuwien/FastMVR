#include <glad/glad.h>
#include "CppUnitTest.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AABB.h"
#include "Scene.h"
#include "VecLine.h"
#include "VecPlane.h"
#include <optional>
#include "GeometryProvider.h"
#include <MathGeoLib.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	static std::vector<glm::vec3> pointsToTestInside = {
			glm::vec3{0,0,0},
			glm::vec3{0,0,1},
			glm::vec3{std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon()},
			glm::vec3{ 19.9999f,  19.9999f,  19.9999f},
			glm::vec3{-19.9999f, -19.9999f, -19.9999f},
		};
	static std::vector<glm::vec3> pointsToTestOutside = {
			glm::vec3{0, 20.1f, 0},
			glm::vec3{std::numeric_limits<float>::max(), 0, 0},
			glm::vec3{ 20.0001f,  20.0001f,  20.0001f},
			glm::vec3{-20.0001f, -20.0001f, -20.0001f},
		};
	static std::vector<float3> pointsToTestInsideFloat3 = {
			float3{0,0,0},
			float3{0,0,1},
			float3{std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon()},
			float3{ 19.9999f,  19.9999f,  19.9999f},
			float3{-19.9999f, -19.9999f, -19.9999f},
		};
	static std::vector<float3> pointsToTestOutsideFloat3 = {
			float3{0, 20.1f, 0},
			float3{std::numeric_limits<float>::max(), 0, 0},
			float3{ 20.0001f,  20.0001f,  20.0001f},
			float3{-20.0001f, -20.0001f, -20.0001f},
		};

	static std::vector<math::AABB> aabbsToTestOutside = {
		math::AABB(float3{-30.0f, -30.0f,    -30.0f}, float3{-20.0001f, -20.0001f, -20.0001f}),
		math::AABB(float3{-30.0f,   0.0f,      0.0f}, float3{-20.0001f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  20.0001f, -10.0f}, float3{100.1f,     30.0001f,   0.0001f}),
		math::AABB(float3{-30.0f,   0.0f,     21.0f}, float3{-20.0001f,  10.0001f,  22.0001f}),
	};
	static std::vector<math::AABB> aabbsToTestInside = {
		math::AABB(float3{-10.0f, -10.0f,    -10.0f}, float3{ 10.0001f,  10.0001f,  10.0001f}),
		math::AABB(float3{-19.999f, 0.0f,      0.0f}, float3{-19.0001f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  10.0001f, -10.0f}, float3{ 10.1f,     19.9999f,   0.0001f}),
		math::AABB(float3{  5.0f,   5.0f,      5.0f}, float3{ 6.f,        6.f,       6.f}),
	};
	static std::vector<math::AABB> aabbsToTestIntersect = {
		math::AABB(float3{-30.0f, -30.0f,    -30.0f}, float3{-19.9999f, -19.9999f, -19.9999f}),
		math::AABB(float3{-30.0f,   0.0f,      0.0f}, float3{-19.9999f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  19.9999f, -10.0f}, float3{100.1f,     30.0001f,   0.0001f}),
		math::AABB(float3{-30.0f,   0.0f,     15.0f}, float3{-10.f,      10.0001f,  22.0001f}),
		math::AABB(float3{-20.01f,-10.0f,    -10.0f}, float3{ 10.0001f,  10.0001f,  10.0001f}),
		math::AABB(float3{-199.f,   0.0f,      0.0f}, float3{-19.9999f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  10.0001f, -20.0f}, float3{ 20.1f,     20.0001f,   0.0001f}),
		math::AABB(float3{ 19.0f,  19.0f,     19.0f}, float3{ 21.f,      21.f,      21.f}),
	};

	static std::vector<math::AABB> aabbsToTestOutsideLargerMargin = {
		math::AABB(float3{-30.0f, -30.0f,    -30.0f}, float3{-21.f, -21.f, -21.f}),
		math::AABB(float3{-30.0f,   0.0f,      0.0f}, float3{-21.f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  21.f, -10.0f}, float3{100.1f,     30.0001f,   0.0001f}),
		math::AABB(float3{-30.0f,   0.0f,     21.0f}, float3{-21.f,  10.0001f,  22.0001f}),
	};
	static std::vector<math::AABB> aabbsToTestIntersectLargerMargin = {
		math::AABB(float3{-30.0f, -30.0f,    -30.0f}, float3{-19.f, -19.f, -19.f}),
		math::AABB(float3{-30.0f,   0.0f,      0.0f}, float3{-19.f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  19.f,-10.0f}, float3{100.1f,     30.0001f,   0.0001f}),
		math::AABB(float3{-30.0f,   0.0f,     15.0f}, float3{-10.f,      10.0001f,  22.0001f}),
		math::AABB(float3{-21.f,-10.0f,    -10.0f}, float3{ 10.0001f,  10.0001f,  10.0001f}),
		math::AABB(float3{-199.f,   0.0f,      0.0f}, float3{-19.f,  10.0001f,  10.0001f}),
		math::AABB(float3{  0.0f,  10.0001f, -20.0f}, float3{ 20.1f,     21.f,   0.0001f}),
		math::AABB(float3{ 19.0f,  19.0f,     19.0f}, float3{ 21.f,      21.f,      21.f}),
	};

	static std::vector<math::Triangle> trianglesToTestOutside = {
		math::Triangle(float3{-40.f, -10.f,  0.f}, float3{-20.0001f,  -5.f, -3.f},     float3{-25.f,  10.f,      2.f}),
		math::Triangle(float3{-25.f,  16.f,  0.f}, float3{-16.f,      25.f, -3.f},     float3{-25.f,  24.f,      2.f}),
		math::Triangle(float3{ 16.f, -20.f, 25.f}, float3{ 25.f,     -20.f, 16.f},     float3{ 25.f,   0.f,     16.f}),
		math::Triangle(float3{ 14.f, -20.01f, 25.f}, float3{ 25.f, -20.01f, 14.f}, float3{ 25.f, -25.f, 14.f}),
		math::Triangle(float3{100.f, 101.f,    102.f}, float3{101.f, 102.f,    100.f}, float3{102.f, 100.f,    101.f}),
	};
	static std::vector<math::Triangle> trianglesToTestInside = { 
		math::Triangle(float3{-19.9999f, -10.f,  0.f},    float3{-17.0f,   -5.f,     -3.f}, float3{-18.f,      10.f,      2.f}),
		math::Triangle(float3{-19.9999f,  16.f,  0.f},    float3{-16.f,    19.9999f, -3.f}, float3{-19.999f,   19.999f,   2.f}),
		math::Triangle(float3{ 16.f, -19.9999f, 19.999f}, float3{19.999f,  19.9999f, 16.f}, float3{ 19.9999f,   0.f,     16.f}),
		math::Triangle(float3{ 14.f, -19.9999f, 19.999f}, float3{19.999f, -19.9999f, 14.f}, float3{ 19.9999f, -19.9999f,  0.f}),
		math::Triangle(float3{ 10.f,  11.f,     12.f},    float3{11.f,     12.f,     10.f}, float3{ 12.f,      10.f,     11.f}),
	};
	static std::vector<math::Triangle> trianglesToTestIntersect = {
		math::Triangle(float3{-40.f, -10.f,  0.f}, float3{-19.9999f,  -5.f, -3.f},     float3{-25.f,  10.f,      2.f}),
		math::Triangle(float3{-25.f,  14.f,  0.f}, float3{-14.f,      25.f, -3.f},     float3{-25.f,  24.f,      2.f}),
		math::Triangle(float3{ 14.f, -20.f, 25.f}, float3{ 25.f,     -20.f, 14.f},     float3{ 25.f,   0.f,     15.5f}),
		math::Triangle(float3{ 16.f, -20.f, 25.f}, float3{ 25.f,     -20.f, 16.f},     float3{ 25.f,   0.f,     10.f}),
		math::Triangle(float3{ 14.f, -19.9999f, 25.f},  float3{ 25.f, -19.9999f, 14.f},  float3{ 25.f, -25.0001f, 14.f}),
		math::Triangle(float3{ 10.f,  11.f,      0.1f}, float3{ 11.f,  12.f,     20.1f}, float3{ 12.f,  10.f,     20.1f}),
		math::Triangle(float3{-25.f,   0.f,      0.f},  float3{ 25.f,   1.f,      2.1f}, float3{ 25.f,   0.f,      1.1f}),
		math::Triangle(float3{-25.f,  25.f,     20.f},  float3{ 30.f,  25.f,     20.f},  float3{ 25.f,  15.f,     15.f}),
	};

	static std::vector<math::Triangle> trianglesToTestOutsideLargerMargins = {
		math::Triangle(float3{-40.f, -10.f,  0.f}, float3{-21.f,  -5.f, -3.f},     float3{-25.f,  10.f,      2.f}),
		math::Triangle(float3{-28.f,  19.f, 18.f}, float3{-19.f,      28.f, 18.f},     float3{-28.f,  24.f,     18.f}),
		math::Triangle(float3{ 19.f, -20.f, 29.f}, float3{ 29.f,     -20.f, 19.f},     float3{ 29.f,   0.f,     19.f}),
		math::Triangle(float3{ 14.f, -21.f, 25.f}, float3{ 25.f, -21.f, 14.f}, float3{ 25.f, -25.f, 14.f}),
		math::Triangle(float3{100.f, 101.f,    102.f}, float3{101.f, 102.f,    100.f}, float3{102.f, 100.f,    101.f}),
	};
	//static std::vector<math::Triangle> trianglesToTestInsideLargerMargins = { 
	//	math::Triangle(float3{-19.f, -10.f,  0.f},    float3{-17.0f,   -5.f,     -3.f}, float3{-18.f,      10.f,      2.f}),
	//	math::Triangle(float3{-19.f,  16.f,  0.f},    float3{-16.f,    19.f, -3.f}, float3{-19.f,   19.f,   2.f}),
	//	math::Triangle(float3{ 16.f, -14.f, 14.f}, float3{14.f,  14.f, 16.f}, float3{ 14.f,   0.f,     16.f}),
	//	math::Triangle(float3{ 14.f, -19.f, 19.f}, float3{19.f, -19.f, 14.f}, float3{ 19.f, -19.f,  0.f}),
	//	math::Triangle(float3{ 10.f,  11.f,     12.f},    float3{11.f,     12.f,     10.f}, float3{ 12.f,      10.f,     11.f}),
	//};
	static std::vector<math::Triangle> trianglesToTestIntersectLargerMargins = {
		math::Triangle(float3{-40.f, -10.f,  0.f}, float3{-19.f,  -5.f, -3.f},     float3{-25.f,  10.f,      2.f}),
		math::Triangle(float3{-25.f,  14.f,  0.f}, float3{-14.f,      25.f, -3.f},     float3{-25.f,  24.f,      2.f}),
		math::Triangle(float3{ 14.f, -20.f, 25.f}, float3{ 25.f,     -20.f, 14.f},     float3{ 25.f,   0.f,     15.5f}),
		math::Triangle(float3{ 16.f, -20.f, 25.f}, float3{ 25.f,     -20.f, 16.f},     float3{ 25.f,   0.f,     10.f}),
		math::Triangle(float3{ 14.f, -19.f, 25.f},  float3{ 25.f, -19.f, 14.f},  float3{ 25.f, -25.0001f, 14.f}),
		math::Triangle(float3{ 10.f,  11.f,      0.1f}, float3{ 11.f,  12.f,     21.f}, float3{ 12.f,  10.f,     21.f}),
		math::Triangle(float3{-25.f,   0.f,      0.f},  float3{ 25.f,   1.f,      2.1f}, float3{ 25.f,   0.f,      1.1f}),
		math::Triangle(float3{-25.f,  25.f,     20.f},  float3{ 30.f,  25.f,     20.f},  float3{ 25.f,  15.f,     15.f}),
	};

	static math::Triangle triangleTranslated(math::Triangle input, float3 translation)
	{
		return math::Triangle{
			float3{input.Vertex(0).x + translation.x, input.Vertex(0).y + translation.y, input.Vertex(0).z + translation.z},
			float3{input.Vertex(1).x + translation.x, input.Vertex(1).y + translation.y, input.Vertex(1).z + translation.z},
			float3{input.Vertex(2).x + translation.x, input.Vertex(2).y + translation.y, input.Vertex(2).z + translation.z}
		};
	}
	
	static math::AABB aabbTranslated(math::AABB input, float3 translation)
	{
		return math::AABB{
			input.minPoint + translation,
			input.maxPoint + translation
		};
	}
	
	TEST_CLASS(AABBTests)
	{
	public:
		TEST_METHOD(CorrectBoundingBoxGeneration)
		{
			std::vector<SceneVertex> vertices = { 
				SceneVertex { glm::vec3{-100,   45,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{ 100,   45,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  53, -100,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  32,  100,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  13,   13, -100}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  99,   54,  100}, glm::vec3{}, 0 },
			};
			auto inQuestion = mymath::AABB::create_for_vertices(vertices);

			auto reference = mymath::AABB::create_for_min_max_coords(glm::vec3(-100.0f, -100.0f, -100.0f), glm::vec3(100.0f, 100.0f, 100.0f));

			Assert::IsTrue(inQuestion == reference);
		}
	};

	TEST_CLASS(VecPlaneTests)
	{
		TEST_METHOD(VecPlaneOrientationAndPointTests_Front)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{ 20, 20, 20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneOrientationAndPointTests_Left)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20,  20, -20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneOrientationAndPointTests_Bottom)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20,-20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneOrientationAndPointTests_Right)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20,  20, 20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneOrientationAndPointTests_Back)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{-20, 20, -20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneOrientationAndPointTests_Top)
		{
			auto plane1 = VecPlane::create_from_points_ccw(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20});
			auto plane2 = VecPlane::create_from_points_ccw(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20,  20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Front)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{ 20, 20, 20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Left)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20,  20, -20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Bottom)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20,-20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Right)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20,  20, 20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Back)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{-20, 20, -20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(VecPlaneNormalizedOrientationAndPointTests_Top)
		{
			auto plane1 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20});
			auto plane2 = VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20,  20});
			auto n1 = plane1.normal();
			auto n2 = plane2.normal();
			auto n1_dot_n2 = glm::dot(glm::normalize(n1), glm::normalize(n2));
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(plane1.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsTrue(plane2.solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
				isOutPlane2 = isOutPlane2 || (plane2.solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		//TEST_METHOD(CartesianFormConversionYieldsSameResults)
		//{
		//	std::array origPlanes = {
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
		//	};

		//	std::vector<VecPlane> convConvPlanes;
		//	for (auto& plane : origPlanes) {
		//		convConvPlanes.push_back(VecPlane::create_from_cartesian_form(plane.cartesian_form()));
		//	}

		//	for (size_t k = 0; k < origPlanes.size(); ++k) {
		//		for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
		//			Assert::IsTrue(origPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for origPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//			Assert::IsTrue(convConvPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for convConvPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//		}
		//	}
		//}

		//TEST_METHOD(CartesianFormConversionFromNormalizedYieldsSameResults)
		//{
		//	std::array origPlanes = {
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
		//	};

		//	std::vector<VecPlane> convConvPlanes;
		//	for (auto& plane : origPlanes) {
		//		convConvPlanes.push_back(VecPlane::create_from_cartesian_form(plane.cartesian_form()));
		//	}

		//	for (size_t k = 0; k < origPlanes.size(); ++k) {
		//		for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
		//			Assert::IsTrue(origPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for origPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//			Assert::IsTrue(convConvPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for convConvPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//		}
		//	}
		//}

		//TEST_METHOD(CartesianFormConversionToNormalizedYieldsSameResults)
		//{
		//	std::array origPlanes = {
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
		//		VecPlane::create_from_points_ccw(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
		//	};

		//	std::vector<VecPlane> convConvPlanes;
		//	for (auto& plane : origPlanes) {
		//		convConvPlanes.push_back(VecPlane::create_from_cartesian_form_normalized(plane.cartesian_form()));
		//	}

		//	for (size_t k = 0; k < origPlanes.size(); ++k) {
		//		for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
		//			Assert::IsTrue(origPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for origPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//			Assert::IsTrue(convConvPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for convConvPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//		}
		//	}
		//}

		//TEST_METHOD(CartesianFormConversionFromNormalizedToNormalizedYieldsSameResults)
		//{
		//	std::array origPlanes = {
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
		//		VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
		//	};

		//	std::vector<VecPlane> convConvPlanes;
		//	for (auto& plane : origPlanes) {
		//		convConvPlanes.push_back(VecPlane::create_from_cartesian_form_normalized(plane.cartesian_form()));
		//	}

		//	for (size_t k = 0; k < origPlanes.size(); ++k) {
		//		for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
		//			Assert::IsTrue(origPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for origPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//			Assert::IsTrue(convConvPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for convConvPlanes[" + std::to_wstring(k) + L"]")) .c_str());
		//		}
		//	}
		//}

		TEST_METHOD(FrustumPlanes_VecPlane_Orientation)
		{
			std::array vecPlanes = {
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
			};
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[0].normalized_normal(), glm::vec3{-1,0,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[0]'s normal points to the left");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[1].normalized_normal(), glm::vec3{ 1,0,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[1]'s normal points to the right");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[2].normalized_normal(), glm::vec3{0, 1,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[2]'s normal points to the top");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[3].normalized_normal(), glm::vec3{0,-1,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[3]'s normal points to the bottom");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[4].normalized_normal(), glm::vec3{0,0, 1}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[4]'s normal points to the near");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[5].normalized_normal(), glm::vec3{0,0,-1}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[5]'s normal points to the far");
			
			auto orthoProjM = glm::ortho(-20.f, 20.f, -20.f, 20.f, -20.f, 20.f);
			auto planes = gamedevs::ExtractPlanesGL(orthoProjM, true);

			Assert::AreEqual(vecPlanes.size(), planes.size());

			for (size_t k = 0; k < vecPlanes.size(); ++k) {
				for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
					Assert::IsTrue(vecPlanes[k].solve_plane_equation_for_position(pointsToTestInside[i]) >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for vecPlanes[" + std::to_wstring(k) + L"]")) .c_str());
					Assert::IsTrue(gamedevs::ClassifyPoint(planes[k], pointsToTestInside[i]) != gamedevs::Halfspace::NEGATIVE, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for planes[" + std::to_wstring(k) + L"]")) .c_str());
				}
				// Outside of any
				bool isOutPlane1 = false;
				bool isOutPlane2 = false;
				for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
					isOutPlane1 = isOutPlane1 || (vecPlanes[k].solve_plane_equation_for_position(pointsToTestOutside[i]) < 0);
					isOutPlane2 = isOutPlane2 || (gamedevs::ClassifyPoint(planes[k], pointsToTestOutside[i]) == gamedevs::Halfspace::NEGATIVE);
				}
				Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of vecPlanes[" + std::to_wstring(k) + L"]").c_str());
				Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of planes[" + std::to_wstring(k) + L"]").c_str());
			}
		}

		TEST_METHOD(PointTestsWithFrustumDirectly)
		{
			auto orthoProjM = glm::ortho(-20.f, 20.f, -20.f, 20.f, -20.f, 20.f);
			auto planes = gamedevs::ExtractPlanesGL(orthoProjM, true);

			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsTrue(gamedevs::IsAnyPointInsideTheFrustum({ pointsToTestInside[i] }, planes), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for frustum")) .c_str());
			}
			// Outside of any
			bool isInsideFrustum = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isInsideFrustum = isInsideFrustum || gamedevs::IsAnyPointInsideTheFrustum({ pointsToTestOutside[i] }, planes);
			}
			Assert::IsFalse(isInsideFrustum, std::wstring(L"any pointsToTestOutside is outside of frustum").c_str());

			Assert::IsTrue(gamedevs::IsAnyPointInsideTheFrustum(pointsToTestInside, planes), L"any pointsToTestInside is inside the frustum");
			Assert::IsFalse(gamedevs::IsAnyPointInsideTheFrustum(pointsToTestOutside, planes), L"all pointsToTestOutside are outside the frustum");
		}
	};

	TEST_CLASS(MathGeoLibTests)
	{
		TEST_METHOD(AabbTests)
		{
			std::vector<SceneVertex> vertices = { 
				SceneVertex { glm::vec3{-100,   45,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{ 100,   45,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  53, -100,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  32,  100,   34}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  13,   13, -100}, glm::vec3{}, 0 },
				SceneVertex { glm::vec3{  99,   54,  100}, glm::vec3{}, 0 },
			};
			std::vector<float3> float3s;
			for (auto& v : vertices) {
				float3s.push_back(float3{v.position.x, v.position.y, v.position.z});
			}

			auto inQuestion = math::AABB::MinimalEnclosingAABB(float3s.data(), float3s.size());
			auto reference = math::AABB{vec{-100.0f, -100.0f, -100.0f}, vec{100.0f, 100.0f, 100.0f}};
			Assert::IsTrue(glm::abs(inQuestion.Volume() - reference.Volume()) < std::numeric_limits<float>::epsilon(), L"AABBs have same volume");
			Assert::IsTrue(glm::abs(inQuestion.MinX() - reference.MinX()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MinX coordinates");
			Assert::IsTrue(glm::abs(inQuestion.MinY() - reference.MinY()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MinY coordinates");
			Assert::IsTrue(glm::abs(inQuestion.MinZ() - reference.MinZ()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MinZ coordinates");
			Assert::IsTrue(glm::abs(inQuestion.MaxX() - reference.MaxX()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MaxX coordinates");
			Assert::IsTrue(glm::abs(inQuestion.MaxY() - reference.MaxY()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MaxY coordinates");
			Assert::IsTrue(glm::abs(inQuestion.MaxZ() - reference.MaxZ()) < std::numeric_limits<float>::epsilon(), L"AABBs have same MaxZ coordinates");
		}

		TEST_METHOD(FrustumMatchesGlmOrtho)
		{
			math::Frustum frust;
			frust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
			frust.SetViewPlaneDistances(0, 40.f);
			frust.SetFrame(float3{0.f, 0.f, 20.f}, float3{0.f, 0.f, -1.f}, float3{0.f, 1.f, 0.f});
			frust.SetOrthographic(40.f, 40.f);

			std::array vecPlanes = {
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, 20, 20}, glm::vec3{-20, -20, -20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{20, 20, -20}, glm::vec3{20, -20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, 20, -20}, glm::vec3{-20, 20, 20}, glm::vec3{20, 20, -20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, -20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{-20, -20, 20}, glm::vec3{20, -20, 20}, glm::vec3{-20, 20, 20}),
				VecPlane::create_from_points_ccw_normalized(glm::vec3{ 20, -20, -20}, glm::vec3{-20, -20, -20}, glm::vec3{ 20, 20, -20})
			};
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[0].normalized_normal(), glm::vec3{-1,0,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[0]'s normal points to the left");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[1].normalized_normal(), glm::vec3{ 1,0,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[1]'s normal points to the right");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[2].normalized_normal(), glm::vec3{0, 1,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[2]'s normal points to the top");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[3].normalized_normal(), glm::vec3{0,-1,0}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[3]'s normal points to the bottom");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[4].normalized_normal(), glm::vec3{0,0, 1}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[4]'s normal points to the near");
			Assert::IsTrue(glm::abs(glm::dot(vecPlanes[5].normalized_normal(), glm::vec3{0,0,-1}) - 1.0f) <= std::numeric_limits<float>::epsilon(), L"vecPlane[5]'s normal points to the far");
			
			auto orthoProjM = glm::ortho(-20.f, 20.f, -20.f, 20.f, -20.f, 20.f);
			auto planes = gamedevs::ExtractPlanesGL(orthoProjM, true);

			Assert::AreEqual(vecPlanes.size(), planes.size());

			std::array<math::Plane, 6> frustPlanes;
			frust.GetPlanes(frustPlanes.data());
			
			for (size_t k = 0; k < vecPlanes.size(); ++k) {
				for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
					auto pt = pointsToTestInside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					auto pleq1 = vecPlanes[k].solve_plane_equation_for_position(pt);
					Assert::IsTrue(pleq1 >= 0, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for vecPlanes[" + std::to_wstring(k) + L"]")) .c_str());
					auto pleq2 = gamedevs::ClassifyPoint(planes[k], pt);
					Assert::IsTrue(pleq2 != gamedevs::Halfspace::NEGATIVE, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for planes[" + std::to_wstring(k) + L"]")) .c_str());
					Assert::IsTrue(!frustPlanes[k].IsOnPositiveSide(ptfl3), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for frustPlanes[" + std::to_wstring(k) + L"]")) .c_str());
					//frustPlanes[k].IsOnPositiveSide(ptfl3)
				}
				// Outside of any
				bool isOutPlane1 = false;
				bool isOutPlane2 = false;
				bool isOutPlane3 = false;
				for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
					auto pt = pointsToTestOutside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					isOutPlane1 = isOutPlane1 || (vecPlanes[k].solve_plane_equation_for_position(pt) < 0);
					isOutPlane2 = isOutPlane2 || (gamedevs::ClassifyPoint(planes[k], pt) == gamedevs::Halfspace::NEGATIVE);
					isOutPlane3 = isOutPlane3 || (frustPlanes[k].IsOnPositiveSide(ptfl3));
				}
				Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of vecPlanes[" + std::to_wstring(k) + L"]").c_str());
				Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of planes[" + std::to_wstring(k) + L"]").c_str());
				Assert::IsTrue(isOutPlane3, std::wstring(L"any pointsToTestOutside is outside of frustPlanes[" + std::to_wstring(k) + L"]").c_str());
			}
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Front)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{-20, -20, 20}, float3{20, -20, 20}, float3{-20, 20, 20});
			plane2.Set(float3{-20, -20, 20}, float3{20, -20, 20}, float3{ 20, 20, 20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Left)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{-20, -20, 20}, float3{-20, 20, 20}, float3{-20, -20, -20});
			plane2.Set(float3{-20, -20, 20}, float3{-20, 20, 20}, float3{-20,  20, -20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Bottom)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{-20, -20, 20}, float3{-20, -20, -20}, float3{ 20, -20, 20});
			plane2.Set(float3{-20, -20, 20}, float3{-20, -20, -20}, float3{ 20, -20,-20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Right)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{ 20, -20, -20}, float3{20, 20, -20}, float3{20, -20, 20});
			plane2.Set(float3{ 20, -20, -20}, float3{20, 20, -20}, float3{20,  20, 20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Back)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{ 20, -20, -20}, float3{-20, -20, -20}, float3{ 20, 20, -20});
			plane2.Set(float3{ 20, -20, -20}, float3{-20, -20, -20}, float3{-20, 20, -20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(MathPlaneOrientationAndPointTests_Top)
		{
			math::Plane plane1;
			math::Plane plane2;
			plane1.Set(float3{-20, 20, -20}, float3{-20, 20, 20}, float3{20, 20, -20});
			plane2.Set(float3{-20, 20, -20}, float3{-20, 20, 20}, float3{20, 20,  20});
			auto n1 = plane1.normal;
			auto n2 = plane2.normal;
			auto n1_dot_n2 = n1.Dot(n2);
			
			Assert::IsTrue(glm::abs(n1_dot_n2 - 1.0f) <= std::numeric_limits<float>::epsilon(), L"plane1 != plane2");

			// Inside of all
			for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
				Assert::IsFalse(plane1.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane1")) .c_str());
				Assert::IsFalse(plane2.IsOnPositiveSide(pointsToTestInsideFloat3[i]), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for plane2")) .c_str());
			}
			// Outside of any
			bool isOutPlane1 = false;
			bool isOutPlane2 = false;
			for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
				isOutPlane1 = isOutPlane1 || (plane1.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
				isOutPlane2 = isOutPlane2 || (plane2.IsOnPositiveSide(pointsToTestOutsideFloat3[i]));
			}
			Assert::IsTrue(isOutPlane1, std::wstring(L"any pointsToTestOutside is outside of plane1").c_str());
			Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of plane2").c_str());
		}

		TEST_METHOD(ConvertExtractedPlanesToMathPlanes)
		{
			auto orthoProjM = glm::ortho(-20.f, 20.f, -20.f, 20.f, -20.f, 20.f);
			auto planes = gamedevs::ExtractPlanesGL(orthoProjM, true);

			std::array<math::Plane, 6> frustPlanes;
			for (size_t i = 0; i < planes.size(); ++i) {
				auto normalVector = float3{planes[i].a, planes[i].b, planes[i].c};
				auto distance = planes[i].d;
				frustPlanes[i] = math::Plane(normalVector, distance);
				// Do some sanity checks right away:
				Assert::IsTrue(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 1.1f));
				Assert::IsTrue(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 100.f));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 0.9f));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(float3{0.f, 0.f, 0.f}));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * -distance));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * -distance * 100.f));
			}
			
			for (size_t k = 0; k < planes.size(); ++k) {
				for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
					auto pt = pointsToTestInside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					auto pleq2 = ClassifyPoint(planes[k], pt);
					Assert::IsTrue(pleq2 != gamedevs::Halfspace::NEGATIVE, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for planes[" + std::to_wstring(k) + L"]")) .c_str());
					Assert::IsTrue(!frustPlanes[k].IsOnPositiveSide(ptfl3), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for frustPlanes[" + std::to_wstring(k) + L"]")) .c_str());
					//frustPlanes[k].IsOnPositiveSide(ptfl3)
				}
				// Outside of any
				bool isOutPlane2 = false;
				bool isOutPlane3 = false;
				for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
					auto pt = pointsToTestOutside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					isOutPlane2 = isOutPlane2 || (ClassifyPoint(planes[k], pt) == gamedevs::Halfspace::NEGATIVE);
					isOutPlane3 = isOutPlane3 || (frustPlanes[k].IsOnPositiveSide(ptfl3));
				}
				Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of planes[" + std::to_wstring(k) + L"]").c_str());
				Assert::IsTrue(isOutPlane3, std::wstring(L"any pointsToTestOutside is outside of frustPlanes[" + std::to_wstring(k) + L"]").c_str());
			}
		}

		TEST_METHOD(ConvertExtractedPlanesToMathPlanesUsing_convert_frustum_planes)
		{
			auto orthoProjM = glm::ortho(-20.f, 20.f, -20.f, 20.f, -20.f, 20.f);
			auto planes = gamedevs::ExtractPlanesGL(orthoProjM, true);

			auto frustPlanes = convert_to_math_frustum_planes(planes);
			
			for (size_t i = 0; i < frustPlanes.size(); ++i) {
				auto normalVector = frustPlanes[i].normal;
				auto distance = frustPlanes[i].d;
				// Do some sanity checks right away:
				Assert::IsTrue(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 1.1f));
				Assert::IsTrue(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 100.f));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * distance * 0.9f));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(float3{0.f, 0.f, 0.f}));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * -distance));
				Assert::IsFalse(frustPlanes[i].IsOnPositiveSide(normalVector * -distance * 100.f));
			}
			
			for (size_t k = 0; k < planes.size(); ++k) {
				for (size_t i = 0; i < pointsToTestInside.size(); ++i) {
					auto pt = pointsToTestInside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					auto pleq2 = ClassifyPoint(planes[k], pt);
					Assert::IsTrue(pleq2 != gamedevs::Halfspace::NEGATIVE, (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for planes[" + std::to_wstring(k) + L"]")) .c_str());
					Assert::IsTrue(!frustPlanes[k].IsOnPositiveSide(ptfl3), (std::wstring(L"pointsToTestInside[") + std::to_wstring(i) + std::wstring(L"] test failed for frustPlanes[" + std::to_wstring(k) + L"]")) .c_str());
					//frustPlanes[k].IsOnPositiveSide(ptfl3)
				}
				// Outside of any
				bool isOutPlane2 = false;
				bool isOutPlane3 = false;
				for (size_t i = 0; i < pointsToTestOutside.size(); ++i) {
					auto pt = pointsToTestOutside[i];
					auto ptfl3 = float3{pt.x, pt.y, pt.z};
					isOutPlane2 = isOutPlane2 || (ClassifyPoint(planes[k], pt) == gamedevs::Halfspace::NEGATIVE);
					isOutPlane3 = isOutPlane3 || (frustPlanes[k].IsOnPositiveSide(ptfl3));
				}
				Assert::IsTrue(isOutPlane2, std::wstring(L"any pointsToTestOutside is outside of planes[" + std::to_wstring(k) + L"]").c_str());
				Assert::IsTrue(isOutPlane3, std::wstring(L"any pointsToTestOutside is outside of frustPlanes[" + std::to_wstring(k) + L"]").c_str());
			}
		}
		TEST_METHOD(FrustumAABBIntersection)
		{
			math::Frustum frust;
			frust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
			frust.SetViewPlaneDistances(0, 40.f);
			frust.SetFrame(float3{0.f, 0.f, 20.f}, float3{0.f, 0.f, -1.f}, float3{0.f, 1.f, 0.f});
			frust.SetOrthographic(40.f, 40.f);

			for (size_t i = 0; i < aabbsToTestInside.size(); ++i) {
				Assert::IsTrue(frust.Intersects(aabbsToTestInside[i]), (std::wstring(L"aabbsToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < aabbsToTestOutside.size(); ++i) {
				Assert::IsFalse(frust.Intersects(aabbsToTestOutside[i]), (std::wstring(L"aabbsToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < aabbsToTestIntersect.size(); ++i) {
				Assert::IsTrue(frust.Intersects(aabbsToTestIntersect[i]), (std::wstring(L"aabbsToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
		}

		TEST_METHOD(FrustumTriangleIntersection)
		{
			math::Frustum frust;
			frust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
			frust.SetViewPlaneDistances(0, 40.f);
			frust.SetFrame(float3{0.f, 0.f, 20.f}, float3{0.f, 0.f, -1.f}, float3{0.f, 1.f, 0.f});
			frust.SetOrthographic(40.f, 40.f);

			// math::Frustum::Intersect uses the Gilbert–Johnson–Keerthi distance algorithm
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(frust.Intersects(trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
				Assert::IsFalse(frust.Intersects(trianglesToTestOutside[i]), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
				Assert::IsTrue(frust.Intersects(trianglesToTestIntersect[i]), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
		}

		TEST_METHOD(AABBtoFrustumTriangleIntersection)
		{
			for (float a = -130.f; a < 130.f; a += 30.f) {
				for (float b = -140.f; b < 140.f; b += 40.f) {
					for (float c = -140.f; c < 150.f; c += 50.f) {
						float3 translation{a, b, c};
						math::AABB aabb = math::AABB::FromCenterAndSize(translation, float3{40.f, 40.f, 40.f});
						math::Frustum frust = convert_to_frustum(aabb);

						// math::Frustum::Intersect uses the Gilbert–Johnson–Keerthi distance algorithm
						for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestInside[i], translation);
							Assert::IsTrue(frust.Intersects(triTra), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestOutside[i], translation);
							Assert::IsFalse(frust.Intersects(triTra), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestIntersect[i], translation);
							Assert::IsTrue(frust.Intersects(triTra), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
					}
				}
			}
		}

		TEST_METHOD(AABBTriangleIntersection)
		{
			for (float a = -131.f; a < 130.f; a += 30.f) {
				for (float b = -143.f; b < 140.f; b += 40.f) {
					for (float c = -147.f; c < 150.f; c += 50.f) {
						float3 translation{a, b, c};
						math::AABB aabb = math::AABB::FromCenterAndSize(translation, float3{40.f, 40.f, 40.f});
			
						for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestInside[i], translation);
							Assert::IsTrue(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestOutside[i], translation);
							Assert::IsFalse(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestIntersect[i], translation);
							Assert::IsTrue(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
					}
				}
			}
		}

		TEST_METHOD(AABBTriangleIntersection2)
		{
			for (float a = -137.f; a < 130.f; a += 30.f) {
				for (float b = -144.f; b < 140.f; b += 40.f) {
					for (float c = -143.f; c < 150.f; c += 50.f) {
						float3 translation{a, b, c};
						math::AABB aabb;
						aabb.minPoint = float3{-20.f, -20.f, -20.f} + translation;
						aabb.maxPoint = float3{ 20.f,  20.f,  20.f} + translation;
						
						for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestInside[i], translation);
							Assert::IsTrue(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestOutside[i], translation);
							Assert::IsFalse(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
							auto triTra = triangleTranslated(trianglesToTestIntersect[i], translation);
							Assert::IsTrue(aabb.Intersects(triTra), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
					}
				}
			}
		}

		TEST_METHOD(ProjectionMatrixFrustumConversion)
		{
			std::array perspMatrices = {
				glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 1000.0f),
				glm::perspective(glm::radians(90.0f), 800.f / 600.f, 0.1f, 10.0f),
				glm::perspective(glm::radians(75.0f), 80000.f / 600.f, 0.1f, 1000.0f),
				glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.0001f, 1000.0f),
			};

			for (size_t k = 0; k < perspMatrices.size(); ++k) {
				auto convertedF = convert_symmetric_perspective_matrix_to_frustum(perspMatrices[k]);
				auto convertedM = convertedF.ProjectionMatrix();
				for (size_t i=0; i < 4; ++i) {
					for (size_t j=0; i < 4; ++i) {
						Assert::IsTrue(std::abs(perspMatrices[k][i][j] - convertedM[i][j]) <= 0.0001f,
							(std::wstring(L"perspMatrices[") + std::to_wstring(k) + std::wstring(L"][") + std::to_wstring(i) + std::wstring(L"][") + std::to_wstring(j) + std::wstring(L"]")).c_str());
					}
				}
			}
		}

		TEST_METHOD(PerspectiveFrustumTriangleIntersection)
		{
			for (float a = -131.f; a < 130.f; a += 30.f) {
				for (float b = -143.f; b < 140.f; b += 40.f) {
					for (float c = -147.f; c < 150.f; c += 50.f) {
						float3 translation{a, b, c};

						auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 980.f, 1020.f);
						auto frust = convert_symmetric_perspective_matrix_to_frustum(pM);
						frust.SetFront(float3{0.f, 0.f, -1.f});
						frust.SetUp(float3{0.f, 1.f, 0.f});
						frust.SetPos(float3{0.f, 0.f, 1000.f} + translation);

						for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
							Assert::IsTrue(frust.Intersects(triangleTranslated(trianglesToTestInside[i], translation)), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestOutsideLargerMargins.size(); ++i) {
							Assert::IsFalse(frust.Intersects(triangleTranslated(trianglesToTestOutsideLargerMargins[i], translation)), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
							Assert::IsTrue(frust.Intersects(triangleTranslated(trianglesToTestIntersectLargerMargins[i], translation)), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}

					}
				}
			}
		}

		TEST_METHOD(PerspectiveFrustumPlanesTriangleIntersection)
		{
			for (float a = -131.f; a < 130.f; a += 30.f) {
				for (float b = -143.f; b < 140.f; b += 40.f) {
					for (float c = -147.f; c < 150.f; c += 50.f) {
						float3 translation{a, b, c};
						glm::vec3 translation_vec3 = convert_to_vec3(translation);

						auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
						auto vM = glm::lookAt(translation_vec3 + glm::vec3{0.f, 0.f, 10020.f}, translation_vec3, glm::vec3{0.f, 1.f, 0.f});
						auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
						auto frustPlanes = convert_to_math_frustum_planes(planes);

						for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
							Assert::IsTrue(is_triangle_in_frustum(frustPlanes, triangleTranslated(trianglesToTestInside[i], translation)), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestOutsideLargerMargins.size(); ++i) {
							Assert::IsFalse(is_triangle_in_frustum(frustPlanes, triangleTranslated(trianglesToTestOutsideLargerMargins[i], translation)), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
							Assert::IsTrue(is_triangle_in_frustum(frustPlanes, triangleTranslated(trianglesToTestIntersectLargerMargins[i], translation)), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}

					}
				}
			}
		}

		TEST_METHOD(PerspectiveFrustumPlanesAABBIntersection)
		{
			int iterationCount = 0;
			for (float a = -131.f; a < 130.f; a += 30.f) {
				for (float b = -143.f; b < 140.f; b += 40.f) {
					for (float c = -147.f; c < 150.f; c += 50.f) {
						iterationCount += 1;
						float3 translation{a, b, c};
						glm::vec3 translation_vec3 = convert_to_vec3(translation);
						
						auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
						auto vM = glm::lookAt(translation_vec3 + glm::vec3{0.f, 0.f, 10020.f}, translation_vec3, glm::vec3{0.f, 1.f, 0.f});
						auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
						auto frustPlanes = convert_to_math_frustum_planes(planes);

						for (size_t i = 0; i < aabbsToTestInside.size(); ++i) {
							Assert::IsTrue(is_aabb_in_frustum(frustPlanes, aabbTranslated(aabbsToTestInside[i], translation)), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < aabbsToTestOutsideLargerMargin.size(); ++i) {
							Assert::IsFalse(is_aabb_in_frustum(frustPlanes, aabbTranslated(aabbsToTestOutsideLargerMargin[i], translation)), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
						for (size_t i = 0; i < aabbsToTestIntersectLargerMargin.size(); ++i) {
							Assert::IsTrue(is_aabb_in_frustum(frustPlanes, aabbTranslated(aabbsToTestIntersectLargerMargin[i], translation)), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
						}
					}
				}
			}
			std::cout << "#iterations in PerspectiveFrustumPlanesAABBIntersection: " << iterationCount << std::endl;
			std::cerr << "#iterations in PerspectiveFrustumPlanesAABBIntersection: " << iterationCount << std::endl;
		}

		TEST_METHOD(AABBPlanesTriangleIntersection)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb);

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
				Assert::IsFalse(is_triangle_in_frustum(aabbPlanes, trianglesToTestOutside[i]), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestIntersect[i]), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
		}

		TEST_METHOD(AABBnormalizedPlanesTriangleIntersection)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb, true);

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestOutside.size(); ++i) {
				Assert::IsFalse(is_triangle_in_frustum(aabbPlanes, trianglesToTestOutside[i]), (std::wstring(L"trianglesToTestOutside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			for (size_t i = 0; i < trianglesToTestIntersect.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestIntersect[i]), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
		}

		TEST_METHOD(Test_does_triangle_really_intersect_one_of_the_frustum_planes)
		{
			auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
			auto vM = glm::lookAt(glm::vec3{0.f, 0.f, 10020.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
			auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
			auto frustPlanes = convert_to_math_frustum_planes(planes);

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsFalse(does_triangle_really_intersect_one_of_the_frustum_planes(frustPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
			// Do not test trianglesToTestOutsideLargerMargins because some of them can intersect one of the frustum planes.
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				Assert::IsTrue(does_triangle_really_intersect_one_of_the_frustum_planes(frustPlanes, trianglesToTestIntersectLargerMargins[i]), (std::wstring(L"trianglesToTestIntersect[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
			}
		}

		TEST_METHOD(ClipInsideTrianglesRemainUnchanged)
		{
			auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
			auto vM = glm::lookAt(glm::vec3{0.f, 0.f, 10020.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
			auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
			auto frustPlanes = convert_to_math_frustum_planes(planes);
			auto frust = convert_symmetric_perspective_matrix_to_frustum(pM);
			frust.SetFront(float3{0.f, 0.f, -1.f});
			frust.SetUp(float3{0.f, 1.f, 0.f});
			frust.SetPos(float3{0.f, 0.f, 10020.f});

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestInside[i], frustPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));
			}

			Assert::AreEqual(trianglesToTestInside.size(), resultingTriangles.size(), L"resultingTriangles.size() not as expected");

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(frustPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(frustPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
				Assert::IsTrue(frust.Intersects(resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using math::Frustum")).c_str());
			}

			// Test if the values haven't changed
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				for (size_t v = 0; v < 3; ++v) {
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).x - resultingTriangles[i].Vertex(v).x) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].x same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).y - resultingTriangles[i].Vertex(v).y) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].y same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).z - resultingTriangles[i].Vertex(v).z) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].z same same")).c_str());
				}
			}
		}

		TEST_METHOD(ClipOutsideTrianglesNothingRemains)
		{
			auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
			auto vM = glm::lookAt(glm::vec3{0.f, 0.f, 10020.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
			auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
			auto frustPlanes = convert_to_math_frustum_planes(planes);
			auto frust = convert_symmetric_perspective_matrix_to_frustum(pM);
			frust.SetFront(float3{0.f, 0.f, -1.f});
			frust.SetUp(float3{0.f, 1.f, 0.f});
			frust.SetPos(float3{0.f, 0.f, 10020.f});

			for (size_t i = 0; i < trianglesToTestOutsideLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestOutsideLargerMargins[i], frustPlanes);
				Assert::IsTrue(clippedTriangles.size() == 0, (std::wstring(L"no triangle remains for trianglesToTestOutsideLargerMargins[") + std::to_wstring(i) + L"]").c_str());
			}

		}

		TEST_METHOD(ClipIntersectingTrianglesRemaindersAreInside)
		{
			auto pM = glm::frustum(-20.f, 20.f, -20.f, 20.f, 10000.f, 10040.f);
			auto vM = glm::lookAt(glm::vec3{0.f, 0.f, 10020.f}, glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
			auto planes = gamedevs::ExtractPlanesGL(pM * vM, true);
			auto frustPlanes = convert_to_math_frustum_planes(planes);
			auto frust = convert_symmetric_perspective_matrix_to_frustum(pM);
			frust.SetFront(float3{0.f, 0.f, -1.f});
			frust.SetUp(float3{0.f, 1.f, 0.f});
			frust.SetPos(float3{0.f, 0.f, 10020.f});

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestIntersectLargerMargins[i], frustPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));

				// resulting triangles must have a smaller area as the input triangles
				for (auto& tri : clippedTriangles) {
					Assert::IsTrue(tri.Area() < trianglesToTestIntersectLargerMargins[i].Area(), (std::wstring(L"Area of a clipped triangle smaller as the source triangle[") + std::to_wstring(i) + L"]").c_str());
				}
			}

			// Test if the remaining triangles are all inside:
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(frustPlanes, trianglesToTestIntersectLargerMargins[i]), (std::wstring(L"trianglesToTestIntersectLargerMargins[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(frustPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
				Assert::IsTrue(frust.Intersects(resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using math::Frustum")).c_str());
			}

		}


		TEST_METHOD(AABBClipInsideTrianglesRemainUnchanged)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb);

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestInside[i], aabbPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));
			}

			Assert::AreEqual(trianglesToTestInside.size(), resultingTriangles.size(), L"resultingTriangles.size() not as expected");

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
			}

			// Test if the values haven't changed
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				for (size_t v = 0; v < 3; ++v) {
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).x - resultingTriangles[i].Vertex(v).x) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].x same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).y - resultingTriangles[i].Vertex(v).y) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].y same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).z - resultingTriangles[i].Vertex(v).z) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].z same same")).c_str());
				}
			}
		}

		TEST_METHOD(AABBClipOutsideTrianglesNothingRemains)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb);

			for (size_t i = 0; i < trianglesToTestOutsideLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestOutsideLargerMargins[i], aabbPlanes);
				Assert::IsTrue(clippedTriangles.size() == 0, (std::wstring(L"no triangle remains for trianglesToTestOutsideLargerMargins[") + std::to_wstring(i) + L"]").c_str());
			}

		}

		TEST_METHOD(AABBClipIntersectingTrianglesRemaindersAreInside)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb);

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestIntersectLargerMargins[i], aabbPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));

				// resulting triangles must have a smaller area as the input triangles
				for (auto& tri : clippedTriangles) {
					Assert::IsTrue(tri.Area() < trianglesToTestIntersectLargerMargins[i].Area(), (std::wstring(L"Area of a clipped triangle smaller as the source triangle[") + std::to_wstring(i) + L"]").c_str());
				}
			}

			// Test if the remaining triangles are all inside:
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestIntersectLargerMargins[i]), (std::wstring(L"trianglesToTestIntersectLargerMargins[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
			}

		}

		TEST_METHOD(AABBnormalizedClipInsideTrianglesRemainUnchanged)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb, true);

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestInside[i], aabbPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));
			}

			Assert::AreEqual(trianglesToTestInside.size(), resultingTriangles.size(), L"resultingTriangles.size() not as expected");

			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestInside[i]), (std::wstring(L"trianglesToTestInside[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
			}

			// Test if the values haven't changed
			for (size_t i = 0; i < trianglesToTestInside.size(); ++i) {
				for (size_t v = 0; v < 3; ++v) {
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).x - resultingTriangles[i].Vertex(v).x) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].x same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).y - resultingTriangles[i].Vertex(v).y) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].y same same")).c_str());
					Assert::IsTrue(std::abs(trianglesToTestInside[i].Vertex(v).z - resultingTriangles[i].Vertex(v).z) < std::numeric_limits<float>::epsilon(), (std::wstring(L"tri[") + std::to_wstring(i) + std::wstring(L"].vtx[") + std::to_wstring(i) + std::wstring(L"].z same same")).c_str());
				}
			}
		}

		TEST_METHOD(AABBnormalizedClipOutsideTrianglesNothingRemains)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb, true);

			for (size_t i = 0; i < trianglesToTestOutsideLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestOutsideLargerMargins[i], aabbPlanes);
				Assert::IsTrue(clippedTriangles.size() == 0, (std::wstring(L"no triangle remains for trianglesToTestOutsideLargerMargins[") + std::to_wstring(i) + L"]").c_str());
			}

		}

		TEST_METHOD(AABBnormalizedClipIntersectingTrianglesRemaindersAreInside)
		{
			math::AABB aabb = math::AABB::FromCenterAndSize(float3{0.f, 0.f, 0.f}, float3{40.f, 40.f, 40.f});
			auto aabbPlanes = get_planes_of_aabb(aabb, true);

			std::vector<math::Triangle> resultingTriangles;
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				auto clippedTriangles = clip_against_frustum_planes(trianglesToTestIntersectLargerMargins[i], aabbPlanes);
				resultingTriangles.insert(std::end(resultingTriangles), std::begin(clippedTriangles), std::end(clippedTriangles));

				// resulting triangles must have a smaller area as the input triangles
				for (auto& tri : clippedTriangles) {
					Assert::IsTrue(tri.Area() < trianglesToTestIntersectLargerMargins[i].Area(), (std::wstring(L"Area of a clipped triangle smaller as the source triangle[") + std::to_wstring(i) + L"]").c_str());
				}
			}

			// Test if the remaining triangles are all inside:
			for (size_t i = 0; i < trianglesToTestIntersectLargerMargins.size(); ++i) {
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, trianglesToTestIntersectLargerMargins[i]), (std::wstring(L"trianglesToTestIntersectLargerMargins[") + std::to_wstring(i) + std::wstring(L"]")).c_str());
				Assert::IsTrue(is_triangle_in_frustum(aabbPlanes, resultingTriangles[i]), (std::wstring(L"resultingTriangles[") + std::to_wstring(i) + std::wstring(L"] using frustum planes")).c_str());
			}

		}
	};

	TEST_CLASS(DoesAABBFrustumIntersectionReallyWork)
	{
	public:

		math::Frustum create_math_frustum(float3 pos, float3 lookAt, float3 upDirection, float nearPlane, float farPlane, float fieldOfView)
		{
			math::Frustum mathFrust;
			mathFrust.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
			auto frontdir = (lookAt - pos).Normalized();
			mathFrust.SetFrame(pos, frontdir, upDirection);
			mathFrust.SetViewPlaneDistances(nearPlane, farPlane);
			mathFrust.SetPerspective(fieldOfView, fieldOfView);
			return mathFrust;
		}
		
		TEST_METHOD(LookatFrustumAABBInside)
		{
			const float far_plane = 60.f;
			const float near_plane = 1.0f;
			const float fov = glm::radians(45.0f);
			glm::mat4 pM = glm::perspective(fov, 1.0f, near_plane, far_plane);
			glm::mat4 mM = glm::mat4(1.0f);

			std::vector<glm::mat4> view_matrices = {
				glm::lookAt(glm::vec3( 5, 7, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 6, 7, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 7, 7, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 8, 7, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 5, 6.5, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 6, 6.5, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 7, 6.5, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
				glm::lookAt(glm::vec3( 8, 6.5, 7),	glm::vec3(0, 5, 0), glm::vec3(0, 1, 0)),
			};

			std::vector<math::Frustum> math_frustums = {
				create_math_frustum(float3( 5, 7, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 6, 7, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 7, 7, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 8, 7, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 5, 6.5, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 6, 6.5, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 7, 6.5, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
				create_math_frustum(float3( 8, 6.5, 7),	float3(0, 5, 0), float3(0, 1, 0), near_plane, far_plane, fov),
			};
			
			// also apply projection- and model-matrix to all of these view-matrices
			std::vector<glm::mat4> proj_view_matrices;
			for (auto& vM : view_matrices) {
				proj_view_matrices.push_back(pM * vM * mM);
			}

			std::vector<std::array<gamedevs::Plane, 6>> extractedPlanes;
			std::vector<std::array<math::Plane, 6>> frustumPlanes;
			for (auto& pvM : proj_view_matrices) {
				extractedPlanes.push_back(gamedevs::ExtractPlanesGL(pvM, true));
				frustumPlanes.push_back(convert_to_math_frustum_planes(extractedPlanes.back()));
			}

			std::vector<math::AABB> aabbs = {
				math::AABB(float3{ -1.0f,  -1.0f,  -1.0f}, float3{ 1.0f,  1.0f,  1.0f}), // fully contained: [0..2]
				math::AABB(float3{ -1.0f,  -1.0f,  -1.0f}, float3{ 0.0f,  0.0f,  0.0f}),
				math::AABB(float3{  0.0f,   0.0f,   0.0f}, float3{ 1.0f,  1.0f,  1.0f}),
				math::AABB(float3{-10.0f, -10.0f, -10.0f}, float3{10.0f, 10.0f, 10.0f}), // partially contained: [3..5] (intersects)
				math::AABB(float3{-10.0f, -10.0f, -10.0f}, float3{ 0.0f,  0.0f,  0.0f}),
				math::AABB(float3{  0.0f,   0.0f,   0.0f}, float3{10.0f, 10.0f, 10.0f}),
			};

			auto direction = float3{0.f, 5.f, 0.f} - float3{5.f, 7.f, 7.f};
			direction.Normalize();
			math::Ray ray{float3{5.f, 7.f, 7.f}, direction.Normalized()};
			math::Plane p{float3{0.f, 1.f, 0.f}, 0.f};
			float d;
			p.Intersects(ray, &d);
			auto intersectionPoint = ray.GetPoint(d);
			Assert::IsTrue(math_frustums[0].Contains(intersectionPoint));
			
			for (float off = near_plane + aabbs[0].Diagonal().Length() * 2.f; off < far_plane; off += 3.0f) {
				// move further away:
				auto translation = ray.GetPoint(off);

				for (size_t a = 0; a < aabbs.size(); ++a) {
					const auto& aabb = aabbs[a];
					auto aabbToTest = aabbTranslated(aabb, translation);

					for (size_t f=0; f < frustumPlanes.size(); ++f) {
						// Test math::Frustum
						Assert::IsTrue(math_frustums[f].Intersects(aabbToTest), (std::wstring(L"aabb[") + std::to_wstring(a) + L"/" + std::to_wstring(off) + L"] at least partially in math::Frustum[" + std::to_wstring(f) + L"]").c_str());
						if (a < 3) {
							Assert::IsTrue(math_frustums[f].Contains(aabbToTest), (std::wstring(L"aabb[") + std::to_wstring(a) + L"/" + std::to_wstring(off) + L"] fully contained in math::Frustum[" + std::to_wstring(f) + L"]").c_str());
						}

						// Test math::Frustum::GetPlanes
						std::array<math::Plane, 6> mathFrustPlanes;
						math_frustums[f].GetPlanes(mathFrustPlanes.data());
						Assert::IsTrue(is_aabb_in_frustum(mathFrustPlanes, aabbToTest), (std::wstring(L"aabb[") + std::to_wstring(a) + L"/" + std::to_wstring(off) + L"] in mathFrustPlanes[" + std::to_wstring(f) + L"]").c_str());

						// Test gamedevs::ExtractPlanesGL
						const auto& frust = frustumPlanes[f];
						Assert::IsTrue(is_aabb_in_frustum(frust, aabbToTest), (std::wstring(L"aabb[") + std::to_wstring(a) + L"/" + std::to_wstring(off) + L"] in frustum[" + std::to_wstring(f) + L"]").c_str());
					}
				}
			}
		}
	};
	
}
