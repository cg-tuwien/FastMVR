#include "Camera.h"

namespace e186
{
	Camera::Camera()
		: m_projection_matrix(1.0f),
		m_projection_config(CameraProjectionConfig_None),
		m_near_plane(0.0f),
		m_far_plane(0.0f),
		m_fov(0.0f),
		m_left(0.0f),
		m_right(0.0f),
		m_top(0.0f),
		m_bottom(0.0f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::set_projection_matrix(glm::mat4 matrix)
	{
		m_projection_config = CameraProjectionConfig_None;
		m_projection_matrix = matrix;
	}

	const glm::mat4& Camera::projection_matrix() const
	{
		return m_projection_matrix;
	}

	glm::mat4 Camera::CalculateViewMatrix()
	{
		glm::vec3 camTarget = GetPosition() + GetFrontVector();
		return glm::lookAt(GetPosition(), camTarget, GetUpVector());
	}

	void Camera::CopyFrom(const Camera& other)
	{
		m_projection_matrix = other.m_projection_matrix;
		m_projection_config = other.m_projection_config;
		m_near_plane = other.m_near_plane;
		m_far_plane = other.m_far_plane;
		m_fov = other.m_fov;
		m_aspect = other.m_aspect;
		m_left = other.m_left;
		m_right = other.m_right;
		m_top = other.m_top;
		m_bottom = other.m_bottom;
		UpdateProjectionMatrix();
	}

	void Camera::SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_projection_config = CameraProjectionConfig_Perspective;
		m_fov = fov;
		m_aspect = aspect;
		m_near_plane = nearPlane;
		m_far_plane = farPlane;
		UpdateProjectionMatrix();
	}

	void Camera::SetOrthogonalProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		m_projection_config = CameraProjectionConfig_Orthogonal;
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
		m_near_plane = nearPlane;
		m_far_plane = farPlane;
		UpdateProjectionMatrix();
	}

	CameraProjectionConfig Camera::projection_config() const
	{
		return m_projection_config;
	}

	float Camera::near_plane() const
	{
		return m_near_plane;
	}

	float Camera::far_plane() const
	{
		return m_far_plane;
	}

	float Camera::field_of_view() const
	{
		return m_fov;
	}

	float Camera::aspect_ratio() const
	{
		return m_aspect;
	}

	float Camera::left() const
	{
		return m_left;
	}

	float Camera::right() const
	{
		return m_right;
	}

	float Camera::top() const
	{
		return m_top;
	}

	float Camera::bottom() const
	{
		return m_bottom;
	}

	void Camera::set_near_plane(float value)
	{
		m_near_plane = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_far_plane(float value)
	{
		m_far_plane = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_field_of_view(float value)
	{
		m_fov = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_aspect_ratio(float value)
	{
		m_aspect = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_left(float value)
	{
		m_left = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_right(float value)
	{
		m_right = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_top(float value)
	{
		m_top = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_bottom(float value)
	{
		m_bottom = value;
		UpdateProjectionMatrix();
	}

	void Camera::UpdateProjectionMatrix()
	{
		switch (m_projection_config)
		{
		case CameraProjectionConfig_None:
			break;
		case CameraProjectionConfig_Perspective:
			m_projection_matrix = glm::perspective(m_fov, m_aspect, m_near_plane, m_far_plane);
			break;
		case CameraProjectionConfig_Orthogonal:
			m_projection_matrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near_plane, m_far_plane);
			break;
		}
	}

	float Camera::CalcZBufferDepth(const glm::vec3& posWS)
	{
		auto posSS = projection_matrix() * CalculateViewMatrix() * glm::vec4(posWS, 1.0f);
		float depth = posSS.z / posSS.w;
		return depth * 0.5f + 0.5f;
	}


	float Camera::CalcZBufferDepth(Transform* transform)
	{
		return CalcZBufferDepth(transform->GetPosition());
	}


}