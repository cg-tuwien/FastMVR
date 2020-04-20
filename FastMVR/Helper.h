#pragma once

#include <cstdint>
#include <GLFW/glfw3.h>
#include <functional>

namespace {
	template<typename T>
	T divup(T a, T b)
	{
		return (a + b - 1) / b;
	}
};

class KeyHelper
{
public:
	KeyHelper(int keycode, GLFWwindow* wnd) : m_keycode{keycode}, m_window{wnd}, m_cur_state{GLFW_RELEASE}, m_last_state{GLFW_RELEASE} {}
	KeyHelper(KeyHelper&&) = default;
	KeyHelper(const KeyHelper&) = delete;
	KeyHelper& operator=(KeyHelper&&) = default;
	KeyHelper& operator=(const KeyHelper&) = delete;
	~KeyHelper() = default;

	void Tick()
	{
		m_last_state = m_cur_state; 
		m_cur_state = glfwGetKey(m_window, m_keycode);
		if (is_down() && m_is_down_handler) { m_is_down_handler(); }
		if (went_down_this_frame() && m_went_down_handler) { m_went_down_handler(); }
		if (went_up_this_frame() && m_went_up_handler) { m_went_up_handler(); }
	}

	bool is_down() { return m_cur_state == GLFW_PRESS; }
	bool is_up() { return m_cur_state == GLFW_RELEASE; }
	bool went_down_this_frame() { return m_cur_state == GLFW_PRESS && m_last_state == GLFW_RELEASE; }
	bool went_up_this_frame() { return m_cur_state == GLFW_RELEASE && m_last_state == GLFW_PRESS; }

	template <typename F>
	KeyHelper&& set_is_down_handler(F handler)
	{
		m_is_down_handler = handler;
		return std::move(*this);
	}

	template <typename F>
	KeyHelper&& set_went_down_handler(F handler)
	{
		m_went_down_handler = handler;
		return std::move(*this);
	}

	template <typename F>
	KeyHelper&& set_went_up_handler(F handler)
	{
		m_went_up_handler = handler;
		return std::move(*this);
	}

public:
	int m_keycode;
	GLFWwindow* m_window;
	int m_cur_state;
	int m_last_state;

	std::function<void()> m_is_down_handler;
	std::function<void()> m_went_down_handler;
	std::function<void()> m_went_up_handler;
};
