#pragma once
#include "ViewTechniquePipeline.h"

struct TechniqueConfig
{
	// How to fundamentally duplicate the geometry
	ViewTechnique::MultiViewDuplication m_duplication_strategy;

	// Where to render into
	ViewTechnique::MultiViewMedium m_type_of_medium;

	// How to switch to the next (part of) medium
	ViewTechnique::MultiViewChange m_view_change_strategy;

	// Use geometry shader pass-through or don't
	ViewTechnique::MultiViewPassThrough m_pass_through_settings;

	// Optional (i.e. if applicable): How to duplicate index buffer geometry
	std::optional<duplication_strategy> m_index_buffer_duplication_strategy;

	// Optional (i.e. if applicable): How many views to produce at the same time, i.e. with one draw call
	std::optional<int> m_max_num_views_to_produce_with_one_draw_call;

	// Optional (i.e. if applicable): How many tiles/viewports/view-targets can a giant texture have as a maximum
	std::optional<int> m_max_num_gtex_tiles;

	// Optional: If set to true, starts measurement BEFORE the call to begin() (in contrast to between begin() and produce())
	std::optional<bool> m_include_begin_in_measurement;

	// Optional: If set to true, does not disable the fragment shader when SHADOW_MAPPING is defined.
	std::optional<bool> m_requires_fragment_shader_for_depth_only;

	TechniqueConfig& setGeometryShaderPassThroughOptimizationToEnabled()
	{
		m_pass_through_settings = ViewTechnique::MultiViewPassThrough::ENABLED;
		return *this;
	}

	TechniqueConfig& setIndexBufferDuplicationStrategy(duplication_strategy strat)
	{
		m_index_buffer_duplication_strategy = strat;
		return *this;
	}
	
	TechniqueConfig& setMaximumNumberOfViewsToProduceWithOneDrawCall(int num)
	{
		m_max_num_views_to_produce_with_one_draw_call = num;
		return *this;
	}

	TechniqueConfig& setMaximumNumberOfTilesInGiantTexture(int tiles)
	{
		assert(m_type_of_medium == ViewTechnique::MultiViewMedium::GIANT_TEXTURE 
			|| m_type_of_medium == ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES
			|| m_type_of_medium == ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES);
		m_max_num_gtex_tiles = tiles;
		return *this;
	}

	bool include_begin_in_measurement() const
	{
		return m_include_begin_in_measurement.value_or(false);
	}

	bool requires_fragment_shader_for_depth_only() const
	{
		return m_requires_fragment_shader_for_depth_only.value_or(false);
	}
};



