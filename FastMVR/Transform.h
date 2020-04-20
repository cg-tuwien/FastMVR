#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform2.hpp>
#include <vector>
#include <string>

namespace e186
{
	/// Helper method to get up-down and left-right rotation angles from a given direction
	glm::vec2 get_angles_from_direction_yaw_pitch(const glm::vec3& direction);

	/// Helper method to get roll and left-right rotation angles from a given direction
	glm::vec2 get_angles_from_direction_roll_pitch(const glm::vec3& direction);
		
	/// extract the translation part out of a matrix
	glm::vec3 get_translation_from_matrix(const glm::mat4& m);

	std::string matrix_to_string(const glm::mat4& m);
	std::string matrix_to_string(const glm::mat3& m);
	std::string matrix_to_string_compact(const glm::mat4& m);
	std::string matrix_to_string_compact(const glm::mat3& m);

	std::string vector_to_string(const glm::vec2& v);
	std::string vector_to_string(const glm::vec3& v);
	std::string vector_to_string(const glm::vec4& v);

	class Transform
	{
	public:
		static const glm::mat4 kIdentityMatrix;
		static const glm::vec3 kFrontVec3;
		static const glm::vec4 kFrontVec4;
		static const glm::vec3 kUpVec3;
		static const glm::vec4 kUpVec4;
		static const glm::vec3 kSideVec3;
		static const glm::vec4 kSideVec4;
		static const glm::vec3 kUnitVec3X;
		static const glm::vec3 kUnitVec3Y;
		static const glm::vec3 kUnitVec3Z;
		static const glm::vec4 kUnitVec4X;
		static const glm::vec4 kUnitVec4Y;
		static const glm::vec4 kUnitVec4Z;

	protected:
		unsigned int m_update_id;		///< increased whenever rotation, translation, scale is altered to determine when matrix-update required

		unsigned int m_query_id;		///< update-id of the matrices; the matrices have to be updated if m_query_id != m_update_id

		glm::mat4 m_rotation;		///< (accumulated) rotation of the object
		glm::vec3 m_translation;	///< (accumulated) translation of the object
		glm::vec3 m_scale;			///< (accumulated) scale of the object

		glm::mat4 m_model_matrix;			///< the model matrix which is calculated on demand

		Transform* m_parent_ptr;				///< a parent Transform
		std::vector<Transform*> m_childs;	///< child Transforms

		friend void AttachTransform(Transform* parent, Transform* child);
		friend void DetachTransform(Transform* parent, Transform* child);

	private:
		void DataUpdated();
		void UpdateMatrices();

	public:
		Transform();
		Transform(glm::vec3 position);
		Transform(Transform&& other) noexcept;
		Transform(const Transform& other) noexcept;
		Transform& operator=(Transform&& other) noexcept;
		Transform& operator=(const Transform& other) noexcept;
		virtual ~Transform();

		void update_from_matrix(glm::mat4 matrix);

		/// sets a new position, current position is overwritten
		void set_position(glm::vec3 position);
		/// sets a new rotation, current rotation is overwritten
		void set_rotation(glm::mat4 rotation);
		/// sets a new scale, current scale is overwritten
		void set_scale(glm::vec3 scale);
		/// sets a new uniform scale, current scale is overwritten
		void set_scale(const float scale);


		/// translates the object from the current position by `translation`
		void Translate(const glm::vec3& translation);
		/// rotates the object by the specified angle around the x-axis
		void RotateX(const float angle);
		/// rotates the object by the specified angle around the y-axis
		void RotateY(const float angle);
		/// rotates the object by the specified angle around the z-axis
		void RotateZ(const float angle);
		/// rotates the object by the specified angle around the specified axis
		void Rotate(const glm::vec3& axis, const float angle);
		/// rotate the object by the specified matrix
		void Rotate(const glm::mat4& mat);
		/// scales the object by the specified scale
		void Scale(const glm::vec3& scale);
		/// scales the object by the specified uniform scale
		void Scale(const float scale);

		/// returns the model matrix, the matrix is updated internally on demand
		virtual glm::mat4 GetModelMatrix();

		glm::mat4 model_matrix();
		glm::vec3 GetLocalFrontVector();

		/// returns a matrix representating rotation only, also regarding parent-transformations
		glm::mat4 GetRotationMatrix();

		/// returns a matrix representing the local rotation, without parent-transformations
		glm::mat4 rotation_matrix();

		/// returns the scale only, also regarding parent-transformations
		glm::vec3 GetScale();

		/// returns the local scale, without parent-transformations
		glm::vec3 scale();

		/// returns the position of the object (which is effectively m_translation)
		glm::vec3 GetPosition();
		/// returns the normalized front-vector, which is FRONT_VEC transformed by the transposed(inverse(modelMatrix))
		glm::vec3 GetFrontVector();
		/// returns the normalized up-vector, which is UP_VEC transformed by the transposed(inverse(modelMatrix))
		glm::vec3 GetUpVector();
		/// returns the cross product of front vector and up vector
		glm::vec3 GetSideVector();

		/// returns the local position, not regarding parent-transformations
		glm::vec3 translation();

		void LookAt(Transform* target);
		void LookAt(const glm::vec3& target);
		void LookAlong(const glm::vec3& direction);

		void AlignUpVectorTowards(Transform* target);
		void AlignUpVectorTowards(const glm::vec3& target);
		void AlignUpVectorAlong(const glm::vec3& direction);

		/// Returns the parent of this Transform, null means no parent
		Transform* parent();
	};

	void AttachTransform(Transform* parent, Transform* child);
	void DetachTransform(Transform* parent, Transform* child);
}
