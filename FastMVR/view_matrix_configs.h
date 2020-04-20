#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>

struct ViewMatrixConfig
{
	const std::string name; //< Attention: Ensure that it is a name which can be used as directory name!!
	const size_t count;
	const std::vector<glm::mat4> matrices;
};

extern std::vector<ViewMatrixConfig> gViewMatrixConfigs;

static void check_view_matrix_config_sanity(const ViewMatrixConfig& _Config)
{
	if (_Config.matrices.size() != _Config.count) {
		throw std::runtime_error(std::string("Problematic ViewMatrixConfig '") + _Config.name + "' -> count[" + std::to_string(_Config.count) + "] does not match vector size[" + std::to_string(_Config.matrices.size()) + "]");
	}
}

static std::vector<std::vector<glm::mat4>> get_all_view_matrix_configs_with_name(std::string _Name)
{
	std::vector<std::vector<glm::mat4>> result;
	
	for (const auto& vmc : gViewMatrixConfigs) {
		if (vmc.name == _Name) {
			check_view_matrix_config_sanity(vmc);
			result.push_back(vmc.matrices);
		}
	}
	
	return result;
}

static const std::vector<glm::mat4>& get_view_matrix_configs_by_index(size_t index)
{
	assert(index < gViewMatrixConfigs.size());
	check_view_matrix_config_sanity(gViewMatrixConfigs[index]);
	return gViewMatrixConfigs[index].matrices;
}

static glm::mat4 calc_view_matrix_with_pos_and_transforms(glm::vec3 curPos, glm::mat4 preTransform, glm::mat4 postTransform)
{
#ifdef LAYERED_RT
	auto M = glm::inverse(postTransform * glm::inverse(glm::lookAtLH(curPos, curPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f))) * preTransform);
#else
	auto M = glm::inverse(postTransform * glm::inverse(glm::lookAt(curPos, curPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f))) * preTransform);
#endif
	return M;
}

static void add_view_matrix_config(std::string name, int matricesX, int matricesY, float offsetX, float offsetY, glm::mat4 preTransform, glm::mat4 postTransform)
{
	std::vector<glm::mat4> matrices;
	matrices.reserve(matricesX * matricesY);
	float startX = (matricesX / 2) * -offsetX;
	float startY = (matricesY / 2) * -offsetY;
	for (int y = 0; y < matricesY; ++y) {
		for (int x = 0; x < matricesX; ++x) {
			auto curPos = glm::vec3(startX + offsetX * static_cast<float>(x), startY + offsetY * static_cast<float>(y), 0.f);
			matrices.push_back(calc_view_matrix_with_pos_and_transforms(curPos, preTransform, postTransform));
		}		
	}
	gViewMatrixConfigs.push_back(ViewMatrixConfig {std::move(name), static_cast<size_t>(matricesX * matricesY), std::move(matrices)});
}

static void add_view_matrix_config(std::string name, int matricesX, int matricesY, float offsetX, float offsetY, std::function<glm::mat4(int, int)> preTransformFu, std::function<glm::mat4(int, int)> postTransformFu)
{
	std::vector<glm::mat4> matrices;
	matrices.reserve(matricesX * matricesY);
	float startX = (matricesX / 2) * -offsetX;
	float startY = (matricesY / 2) * -offsetY;
	for (int y = 0; y < matricesY; ++y) {
		for (int x = 0; x < matricesX; ++x) {
			auto curPos = glm::vec3(startX + offsetX * static_cast<float>(x), startY + offsetY * static_cast<float>(y), 0.f);
			glm::mat4 preTransform(1.0f);
			glm::mat4 postTransform(1.0f);
			if (preTransformFu) { preTransform = preTransformFu(x, y); }
			if (postTransformFu) { postTransform = postTransformFu(x, y); }
			matrices.push_back(calc_view_matrix_with_pos_and_transforms(curPos, preTransform, postTransform));
		}		
	}
	gViewMatrixConfigs.push_back(ViewMatrixConfig {std::move(name), static_cast<size_t>(matricesX * matricesY), std::move(matrices)});
}
