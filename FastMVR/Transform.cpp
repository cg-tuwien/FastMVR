#include "Transform.h"

namespace e186
{
	const glm::mat4 Transform::kIdentityMatrix(1.0f);
	const glm::vec3 Transform::kFrontVec3(0.0f, 0.0f, -1.0f);
	const glm::vec4 Transform::kFrontVec4(0.0f, 0.0f, -1.0f, 1.0f);
	const glm::vec3 Transform::kUpVec3(0.0f, 1.0f, 0.0f);
	const glm::vec4 Transform::kUpVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec3 Transform::kSideVec3(1.0f, 0.0f, 0.0f);
	const glm::vec4 Transform::kSideVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec3 Transform::kUnitVec3X(1, 0, 0);
	const glm::vec3 Transform::kUnitVec3Y(0, 1, 0);
	const glm::vec3 Transform::kUnitVec3Z(0, 0, 1);
	const glm::vec4 Transform::kUnitVec4X(1, 0, 0, 1);
	const glm::vec4 Transform::kUnitVec4Y(0, 1, 0, 1);
	const glm::vec4 Transform::kUnitVec4Z(0, 0, 1, 1);

	glm::vec2 get_angles_from_direction_yaw_pitch(const glm::vec3& direction)
	{
		glm::vec3 floorProjection(direction.x, 0.0f, direction.z);
		float directionLength = glm::length(floorProjection);
		float updownAngle = glm::atan(direction.y, directionLength);	
		float leftrightAngle = -glm::atan(direction.x, -direction.z);
		return glm::vec2(leftrightAngle, updownAngle);
	}

	glm::vec2 get_angles_from_direction_roll_pitch(const glm::vec3& direction)
	{
		glm::vec3 wallProjection(direction.x, direction.y, 0.0f);
		float directionLength = glm::length(wallProjection);
		float rollAngle = glm::atan(direction.z, directionLength);		
		float leftrightAngle = -glm::atan(direction.x, direction.y);	
		return glm::vec2(leftrightAngle, rollAngle);
	}


	std::string matrix_to_string(const glm::mat4& m)
	{
		char buf[128];
		sprintf_s(buf, 128, 
			"\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n",
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
		return buf;
	}

	std::string matrix_to_string(const glm::mat3& m)
	{
		char buf[128];
		sprintf_s(buf, 128, 
			"\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n",
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]);
		return buf;
	}

	std::string matrix_to_string_compact(const glm::mat4& m)
	{
		char buf[128];
		sprintf_s(buf, 128, 
			"{{%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}}\n",
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
		return buf;
	}

	std::string matrix_to_string_compact(const glm::mat3& m)
	{
		char buf[128];
		sprintf_s(buf, 128, 
			"{{%.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f}}\n",
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]);
		return buf;
	}


	std::string vector_to_string(const glm::vec2& v)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f)", v.x, v.y);
		return buf;
	}

	std::string vector_to_string(const glm::vec3& v)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f, %.2f)", v.x, v.y, v.z);
		return buf;
	}

	std::string vector_to_string(const glm::vec4& v)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f, %.2f, %.2f)", v.x, v.y, v.z, v.w);
		return buf;
	}


	Transform::Transform() :
		m_update_id(0),
		m_query_id(0),
		m_rotation(1.0f), // rotation is a matrix
		m_translation(0.0f),
		m_scale(1.0f),
		m_model_matrix(1.0f),
		m_parent_ptr(nullptr)
	{
	}

	Transform::Transform(glm::vec3 position) :
		m_update_id(0),
		m_query_id(0),
		m_rotation(1.0f), // rotation is a matrix
		m_translation(std::move(position)),
		m_scale(1.0f),
		m_model_matrix(1.0f),
		m_parent_ptr(nullptr)
	{
	}

	Transform::Transform(Transform&& other) noexcept :
		m_update_id(other.m_update_id),
		m_query_id(other.m_query_id),
		m_rotation(std::move(other.m_rotation)),
		m_translation(std::move(other.m_translation)),
		m_scale(std::move(other.m_scale)),
		m_model_matrix(std::move(other.m_model_matrix)),
		m_parent_ptr(std::move(other.m_parent_ptr)),
		m_childs(std::move(other.m_childs))
	{
		other.m_update_id = 0;
		other.m_query_id = 0;
		other.m_parent_ptr = nullptr;
		other.m_childs.clear();
	}

	Transform::Transform(const Transform& other) noexcept :
		m_update_id(other.m_update_id),
		m_query_id(other.m_query_id),
		m_rotation(other.m_rotation),
		m_translation(other.m_translation),
		m_scale(other.m_scale),
		m_model_matrix(other.m_model_matrix),
		m_parent_ptr(other.m_parent_ptr),
		m_childs(other.m_childs)
	{
	}

	Transform& Transform::operator=(Transform&& other) noexcept
	{
		m_update_id = other.m_update_id;
		m_query_id = other.m_query_id;
		m_rotation = std::move(other.m_rotation);
		m_translation = std::move(other.m_translation);
		m_scale = std::move(other.m_scale);
		m_model_matrix = std::move(other.m_model_matrix);
		m_parent_ptr = std::move(other.m_parent_ptr);
		m_childs = std::move(other.m_childs);
		other.m_update_id = 0;
		other.m_query_id = 0;
		other.m_parent_ptr = nullptr;
		other.m_childs.clear();
		return *this;
	}

	Transform& Transform::operator=(const Transform& other) noexcept
	{
		m_update_id = other.m_update_id;
		m_query_id = other.m_query_id;
		m_rotation = other.m_rotation;
		m_translation = other.m_translation;
		m_scale = other.m_scale;
		m_model_matrix = other.m_model_matrix;
		m_parent_ptr = other.m_parent_ptr;
		m_childs = other.m_childs;
		return *this;
	}

	Transform::~Transform()
	{
	}

	void Transform::update_from_matrix(glm::mat4 matrix)
	{
		auto translation = glm::vec3{matrix[3]};
		auto scale = glm::vec3 {
			glm::length(glm::vec3{matrix[0]}),
			glm::length(glm::vec3{matrix[1]}),
			glm::length(glm::vec3{matrix[2]})
		};
		auto rotation = glm::mat4{
			matrix[0] / scale.x,
			matrix[1] / scale.y,
			matrix[2] / scale.z,
			glm::vec4{0.f, 0.f, 0.f, 1.f}
		};
		
		m_translation = translation;
		m_scale = scale;
		m_rotation = rotation;
	}

	void Transform::DataUpdated()
	{
		++m_update_id;
	}

	void Transform::UpdateMatrices()
	{
		if (m_update_id != m_query_id)
		{
			//                                                      3.          2.        1.
			// calculate model-matrix, multiplication order is translation * rotation * scale
			m_model_matrix = glm::translate(m_translation);
			m_model_matrix = m_model_matrix * m_rotation;
			m_model_matrix = glm::scale(m_model_matrix, m_scale);

			m_query_id = m_update_id;
		}
	}


	// Set transformations

	void Transform::set_position(glm::vec3 position)
	{
		m_translation = std::move(position);
		DataUpdated();
	}

	void Transform::set_rotation(glm::mat4 rotation)
	{
		m_rotation = std::move(rotation);
		DataUpdated();
	}

	void Transform::set_scale(glm::vec3 scale)
	{
		m_scale = std::move(scale);
		DataUpdated();
	}

	void Transform::set_scale(const float scale)
	{
		m_scale = glm::vec3(scale);
		DataUpdated();
	}


	// Alter current transformations 

	void Transform::Translate(const glm::vec3& translation)
	{
		m_translation += translation;
		DataUpdated();
	}

	void Transform::Rotate(const glm::vec3& axis, const float angle)
	{
		m_rotation = glm::rotate(angle, axis) * m_rotation;
		DataUpdated();
	}

	void Transform::Rotate(const glm::mat4& mat)
	{
		m_rotation = mat * m_rotation;
		DataUpdated();
	}

	void Transform::RotateX(const float angle)
	{
		Rotate(kUnitVec3X, angle);
	}

	void Transform::RotateY(const float angle)
	{
		Rotate(kUnitVec3Y, angle);
	}

	void Transform::RotateZ(const float angle)
	{
		Rotate(kUnitVec3Z, angle);
	}

	void Transform::Scale(const glm::vec3& scale)
	{
		m_scale += scale;
		DataUpdated();
	}

	void Transform::Scale(const float scale)
	{
		m_scale += glm::vec3(scale);
		DataUpdated();
	}



	// Query matrices

	glm::mat4 Transform::GetModelMatrix()
	{
		UpdateMatrices();
		if (m_parent_ptr)
		{
			return m_parent_ptr->GetModelMatrix() * m_model_matrix;
		}
		else
			return m_model_matrix;
	}



	glm::mat4 Transform::model_matrix()
	{
		return m_model_matrix;
	}

	glm::vec3 Transform::GetLocalFrontVector()
	{
		glm::mat4 mM = model_matrix();
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kFrontVec4));
	}

	glm::vec3 get_translation_from_matrix(const glm::mat4& m)
	{
		return glm::vec3(m[3][0], m[3][1], m[3][2]);
	}


	glm::mat4 Transform::GetRotationMatrix()
	{
		if (m_parent_ptr)
			return m_parent_ptr->GetRotationMatrix() * m_rotation;
		else
			return m_rotation;
	}

	glm::mat4 Transform::rotation_matrix()
	{
		return m_rotation;
	}


	glm::vec3 Transform::GetScale()
	{
		if (m_parent_ptr)
			return m_parent_ptr->GetScale() * m_scale;
		else
			return m_scale;
	}

	glm::vec3 Transform::scale()
	{
		return m_scale;
	}


	// query position and orientation-vectors

	glm::vec3 Transform::translation()
	{
		return m_translation;
	}

	glm::vec3 Transform::GetPosition()
	{
		return  get_translation_from_matrix(GetModelMatrix());
	}

	glm::vec3 Transform::GetFrontVector()
	{
		glm::mat4 mM = GetModelMatrix();
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kFrontVec4));
	}

	glm::vec3 Transform::GetUpVector()
	{
		glm::mat4 mM = GetModelMatrix();
		glm::mat4 itM = glm::inverseTranspose(mM);
		return glm::normalize(glm::vec3(itM * kUpVec4));
	}

	glm::vec3 Transform::GetSideVector()
	{
		return glm::cross(GetFrontVector(), GetUpVector());
	}


	void Transform::LookAt(Transform* target)
	{
		assert(target);
		LookAt(target->GetPosition());
	}

	void Transform::LookAt(const glm::vec3& target)
	{
		glm::vec3 direction = glm::normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Y) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void Transform::LookAlong(const glm::vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_yaw_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Y) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}


	void Transform::AlignUpVectorTowards(Transform* target)
	{
		assert(target);
		AlignUpVectorTowards(target->GetPosition());
	}

	void Transform::AlignUpVectorTowards(const glm::vec3& target)
	{
		glm::vec3 directon = glm::normalize(target - GetPosition());
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(directon);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Z) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}

	void Transform::AlignUpVectorAlong(const glm::vec3& direction)
	{
		glm::vec2 anglesToTarget = get_angles_from_direction_roll_pitch(direction);
		glm::mat4 rotationToTarget = glm::rotate(anglesToTarget.x, kUnitVec3Z) * glm::rotate(anglesToTarget.y, kUnitVec3X);
		set_rotation(rotationToTarget);
	}



	Transform* Transform::parent()
	{
		return m_parent_ptr;
	}


	void AttachTransform(Transform* parent, Transform* child)
	{
		assert(parent);
		assert(child);

		child->m_parent_ptr = parent;

		auto result = std::find(parent->m_childs.begin(), parent->m_childs.end(), child);
		if (parent->m_childs.end() == result)
			parent->m_childs.push_back(child);
		else
			printf("In AttachTransform: Prevented double-adding of child[%p] to parent[%p]\n", child, parent);
	}

	void DetachTransform(Transform* parent, Transform* child)
	{
		assert(parent);
		assert(child);

		child->m_parent_ptr = nullptr;
		auto it = parent->m_childs.begin();
		while (it != parent->m_childs.end())
		{
			if (*it == child)
				break;
		}

		if (it != parent->m_childs.end())
			parent->m_childs.erase(it);
		else
			printf("In DetachTransform: Couldn't find the Transform[%p] in the parent's[%p] m_childs\n", child, parent);
	}
}
