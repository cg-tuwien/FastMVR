#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "Helper.h"

namespace e186
{
	class QuakeCamera : public Camera
	{
	public:
		QuakeCamera(GLFWwindow* window);
		~QuakeCamera();

		void HandleInputOnly();
		void Update(double deltaTime);

		float rotation_speed() const { return m_rotation_speed; }
		float move_speed() const { return m_move_speed; }
		float fast_multiplier() const { return m_fast_multiplier; }
		float slow_multiplier() const { return m_slow_multiplier; }

		void set_rotation_speed(float value) { m_rotation_speed = value; }
		void set_move_speed(float value) { m_move_speed = value; }
		void set_fast_multiplier(float value) { m_fast_multiplier = value; }
		void set_slow_multiplier(float value) { m_slow_multiplier = value; }

		void align_to_rotation_matrix()
		{
			const auto front = glm::vec3{0.f, 0.f, -1.f};
			const auto off = glm::mat3{m_rotation} * front;
			const auto nrmOff = glm::normalize(off);
			
			const auto horiz = glm::normalize(glm::vec3{off.x, 0.f, off.z});
			const auto dotHoriz = glm::dot(front, horiz);
			const auto crossHoriz = glm::cross(front, horiz);
			const auto horizSign = glm::sign(crossHoriz.y);
			const auto angleHoriz = horizSign * glm::acos( dotHoriz );

			const auto dotVerti = glm::clamp(glm::dot(horiz, nrmOff), -1.0f, 1.0f);
			const auto rotAxis = glm::cross(horiz, nrmOff);
			const auto crossVerti = glm::cross(rotAxis, horiz); // still on xz-plane
			const auto vertSign = glm::sign(crossVerti.y);
			const auto angleVerti = vertSign * glm::acos( dotVerti );
			
			m_accumulated_mouse_movement = glm::vec2{angleHoriz, angleVerti};
		}

		void switch_mode();
		
	private:
		void AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime);
		void AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime);

		float m_rotation_speed;
		float m_move_speed;
		float m_fast_multiplier;
		float m_slow_multiplier;

		glm::vec2 m_accumulated_mouse_movement;
		bool m_capture_input;

		GLFWwindow* m_window;

		KeyHelper m_LShift;
		KeyHelper m_LCtrl;
		KeyHelper m_Tab;
		KeyHelper m_RCtrl;
		KeyHelper m_I;
		KeyHelper m_W;
		KeyHelper m_S;
		KeyHelper m_D;
		KeyHelper m_A;
		KeyHelper m_Q;
		KeyHelper m_E;
	};
}
