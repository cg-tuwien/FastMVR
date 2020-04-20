#include "QuakeCamera.h"

namespace e186
{
	QuakeCamera::QuakeCamera(GLFWwindow* window) : 
		m_rotation_speed(0.001f),
		m_move_speed(4.5f), // 4.5 m/s
		m_fast_multiplier(6.0f), // 27 m/s
		m_slow_multiplier(0.2f), // 0.9 m/s
		m_accumulated_mouse_movement(0.0f, 0.0f),
		m_window(window),
		m_LShift(GLFW_KEY_LEFT_SHIFT, window),
		m_LCtrl(GLFW_KEY_LEFT_CONTROL, window),
		m_Tab(GLFW_KEY_TAB, window),
		m_RCtrl(GLFW_KEY_RIGHT_CONTROL, window),
		m_I('I', window),
		m_W('W', window),
		m_S('S', window),
		m_D('D', window),
		m_A('A', window),
		m_Q('Q', window),
		m_E('E', window)
	{
	}

	QuakeCamera::~QuakeCamera()
	{
	}

	void QuakeCamera::AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		glm::vec3 rotatedVector = glm::vec3(m_rotation * homoVectorToAdd);
		float speedMultiplier = 1.0f;
		if (m_LShift.is_down())
		{
			speedMultiplier = m_fast_multiplier;
		}
		if (m_LCtrl.is_down())
		{
			speedMultiplier = m_slow_multiplier;
		}
		Translate(m_move_speed * speedMultiplier * static_cast<float>(deltaTime) * rotatedVector);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		float speedMultiplier = 1.0f;
		if (m_LShift.is_down())
		{
			speedMultiplier = m_fast_multiplier;
		}
		if (m_LCtrl.is_down())
		{
			speedMultiplier = m_slow_multiplier;
		}
		Translate(m_move_speed * speedMultiplier * static_cast<float>(deltaTime) * homoVectorToAdd);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::switch_mode()
	{
		const auto wnd = m_window;
		int width, height;
		glfwGetWindowSize(m_window, &width, &height);
		
		auto newInputMode = GLFW_CURSOR_HIDDEN == glfwGetInputMode(wnd, GLFW_CURSOR) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN;
		glfwSetInputMode(wnd, GLFW_CURSOR, newInputMode);
		m_capture_input = GLFW_CURSOR_HIDDEN == newInputMode;
		glfwSetCursorPos(wnd, width / 2.0, height / 2.0);
	}
	
	void QuakeCamera::HandleInputOnly()
	{
		// switch mode
		if (m_Tab.went_down_this_frame())
		{
			switch_mode();
		}

		// display info
		if (m_I.went_down_this_frame()	&& (m_LCtrl.is_down() || m_RCtrl.is_down()))
		{
			printf("QuakeCamera's position: %s\n", vector_to_string(GetPosition()).c_str());
			printf("QuakeCamera's view-dir: %s\n", vector_to_string(GetFrontVector()).c_str());
			printf("QuakeCamera's up-vec:   %s\n\n", vector_to_string(GetUpVector()).c_str());
		}
	}

	void QuakeCamera::Update(double delta_time)
	{
		m_LShift.Tick();
		m_LCtrl.Tick();
		m_Tab.Tick();
		m_RCtrl.Tick();
		m_I.Tick();
		m_W.Tick();
		m_S.Tick();
		m_D.Tick();
		m_A.Tick();
		m_Q.Tick();
		m_E.Tick();

		HandleInputOnly();

		if (!m_capture_input)
		{
			glm::mat4 cameraRotation = glm::rotate(m_accumulated_mouse_movement.x, kUnitVec3Y) * glm::rotate(m_accumulated_mouse_movement.y, kUnitVec3X);
			set_rotation(cameraRotation);
			return;
		}

		const auto wnd = m_window;
		int width, height;
		glfwGetWindowSize(m_window, &width, &height);

		// query the position of the mouse cursor
		double mouse_x, mouse_y;
		glfwGetCursorPos(wnd, &mouse_x, &mouse_y);
		
		// calculate how much the cursor has moved from the center of the screen
		glm::ivec2 mouseMoved = glm::ivec2(width / 2 - mouse_x, height / 2 - mouse_y);

		// accumulate values and create rotation-matrix
		m_accumulated_mouse_movement.x += m_rotation_speed * static_cast<float>(mouseMoved.x);
		m_accumulated_mouse_movement.y += m_rotation_speed * static_cast<float>(mouseMoved.y);
		m_accumulated_mouse_movement.y = glm::clamp(m_accumulated_mouse_movement.y, -glm::half_pi<float>(), glm::half_pi<float>());
		glm::mat4 cameraRotation = glm::rotate(m_accumulated_mouse_movement.x, kUnitVec3Y) * glm::rotate(m_accumulated_mouse_movement.y, kUnitVec3X);

		// set the rotation
		set_rotation(cameraRotation);

		// move camera to new position
		if (m_W.is_down())
			AddToCameraPositionRelative( kFrontVec4, delta_time);
		if (m_S.is_down())
			AddToCameraPositionRelative(-kFrontVec4, delta_time);
		if (m_D.is_down())
			AddToCameraPositionRelative( kSideVec4, delta_time);
		if (m_A.is_down())
			AddToCameraPositionRelative(-kSideVec4, delta_time);
		if (m_Q.is_down())
			AddToCameraPositionAbsolute(-kUpVec4, delta_time);
		if (m_E.is_down())
			AddToCameraPositionAbsolute( kUpVec4, delta_time);

		// reset the mouse-cursor to the center of the screen
		glfwSetCursorPos(wnd, width / 2.0, height / 2.0);
	}


}
