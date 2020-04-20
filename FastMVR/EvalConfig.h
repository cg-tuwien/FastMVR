#pragma once

#include <cstdint>
#include <glm/matrix.hpp>
#include <vector>

struct EvalConfig
{
	enum class DepthSplitFunction { DISCRETE };

	uint32_t resolution_x = 0;
	uint32_t resolution_y = 0;
	uint32_t depth_copy_interrupt_interval = 1;
	uint32_t view_copy_interrupt_interval = 1;
	DepthSplitFunction split_function = DepthSplitFunction::DISCRETE;

	std::vector<std::vector<glm::mat4>> orig_sets_of_matrices;
	std::vector<std::vector<glm::mat4>> sets_of_matrices;
	glm::mat4 projectionMatrix;
	std::vector<glm::vec2> depth_ranges;

	void verify_equal_number_of_matrices_per_view_position() const
	{
		assert(sets_of_matrices.size() > 0);
		for (const auto& set : sets_of_matrices) {
			assert(set.size() == sets_of_matrices[0].size());
		}
		assert(orig_sets_of_matrices.size() > 0);
		for (const auto& set : orig_sets_of_matrices) {
			assert(set.size() == orig_sets_of_matrices[0].size());
		}
	}
	
	size_t number_of_matrices_per_view_position() const
	{
		assert(!sets_of_matrices.empty());
		return sets_of_matrices[0].size();
	}
};