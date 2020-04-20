#pragma once

#include "Transform.h"

namespace e186
{
	enum CameraProjectionConfig
	{
		CameraProjectionConfig_None,
		CameraProjectionConfig_Perspective,
		CameraProjectionConfig_Orthogonal,
	};

	class Camera : public Transform
	{
	protected:
		glm::mat4 m_projection_matrix;

		CameraProjectionConfig m_projection_config;
		float m_near_plane;
		float m_far_plane;
		float m_fov;	// perspective only
		float m_aspect; // perspective only
		float m_left;	// ortho only
		float m_right;	// ortho only
		float m_top;	// ortho only
		float m_bottom; // ortho only

	public:
		Camera();
		Camera(Camera&&) noexcept = default;
		Camera(const Camera&) noexcept = default;
		Camera& operator=(Camera&&) noexcept = default;
		Camera& operator=(const Camera&) noexcept = default;
		virtual ~Camera();

		/// returns the projection matrix
		const glm::mat4& projection_matrix() const;

		void set_projection_matrix(glm::mat4 matrix);
		void SetPerspectiveProjection(float fov, float aspect, float near, float far);
		void SetOrthogonalProjection(float left, float right, float bottom, float top, float near, float far);
		CameraProjectionConfig projection_config() const;
		float near_plane() const;
		float far_plane() const;
		float field_of_view() const;
		float aspect_ratio() const;
		float left() const;
		float right() const;
		float top() const;
		float bottom() const;
		void set_near_plane(float value);
		void set_far_plane(float value);
		void set_field_of_view(float value);
		void set_aspect_ratio(float value);
		void set_left(float value);
		void set_right(float value);
		void set_top(float value);
		void set_bottom(float value);

		/// calculates the z-buffer depth of the specified position in world space
		float CalcZBufferDepth(const glm::vec3& posWS);
		/// Calculates the z-buffer depth of the specified GameObject's position 
		float CalcZBufferDepth(Transform* transform);

		/// calculates the view matrix based on position, front- and up-vectors;
		/// a lookat-calculation is performed
		glm::mat4 CalculateViewMatrix();

		void CopyFrom(const Camera& other);

	private:
		void UpdateProjectionMatrix();
	};
}