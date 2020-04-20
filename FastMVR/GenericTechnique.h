#pragma once
#include "RenderPipeline.h"
#include "TechniqueConfig.h"
#include "EvalConfig.h"
#include "RenderResourcesGL.h"
#include <array>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include "AdditionalShaderCode.h"


class GenericTechnique : public RenderPipeline
{
public:
	using ChangeT = ViewTechnique::MultiViewChange;
	using MediumT = ViewTechnique::MultiViewMedium;
	
	/** Constructor for a `GenericTechnique`
	 *	@param	name		The (human readable/understandable/unique) name of this technique
	 *	@param	technCfg	This technique's configuration parameters which determine the GL-calls 
	 *						etc. that this instance of `GenericTechnique` will isse.
	 *						There are a lot of switch-statements which switch on these parameters.
	 *	@param	vert		Vertex Shader Code to be used with this technique's graphics pipeline
	 *	@param	geom		Geometry Shader Code to be used with this technique's graphics pipeline
	 *	@param	frag		Fragment Shader Code to be used with this technique's graphics pipeline
	 *	@param	shaderAdd	Additional shader code which shall be inserted into the shader source code
	 *						before compiling them. The code will always be inserted right after the
	 *						"#version" line into the shader files (or at the top if not found).
	 *						Hint: Use the convenience method `add_code` and the methods of `struct AdditionalShaderCode`. 
	 */
	GenericTechnique(std::string name, TechniqueConfig technCfg, EvalConfig evCfg, GeometryProviderBase& geometry, std::string vert, std::string geom, std::string frag, AdditionalShaderCode shaderAdd = {});
	~GenericTechnique();
	void init(RenderResources* resources) override;
	void begin() override;
	void produceLevel(uint32_t level, ChangeT viewChangeType, MediumT viewMediumType, uint32_t viewStartIndex, const std::vector<glm::mat4>& mats);
	void produce() override;
	void end() override;
	void dumpImages(const char* directory) override;
	int get_view_matrices_count() const override;
	glm::mat4 get_view_proj_matrix(int i) override;
	glm::mat4 get_view_matrix(int i) override;
	glm::mat4 get_proj_matrix(int i) override;
	const std::vector<glm::mat4>& get_all_view_proj_matrices() override;
	// get view-proj matrices from the given range: [beginIdx, endIdx)
	std::vector<glm::mat4> get_view_proj_matrices_range(int beginIdx, int endIdx);
	const glm::mat4* get_view_proj_matrices_data_ptr() override;
	TextureAndLayer get_nth_result_texture_info(int index, int range = 0) override;
	TextureAndLayer get_nth_depth_texture_info(int index, int range = 0) override;
	glm::ivec2 get_nth_result_offsets(int index) override;
	glm::ivec2 get_nth_result_range(int index) override;

	inline void save_resources_ptr(RenderResources* ptr) 
	{ 
		assert(nullptr != dynamic_cast<RenderResourcesGL*>(ptr)); 
		m_resources = ptr; 
	}

	bool has_shader_uniform(std::string name, GLenum dataType);
	bool has_shader_uniform_block(std::string name);
	bool shader_source_contains(std::string name);

	RenderResources::ViewportConfig calculate_gianttex_viewportconfig(int matrixId);
	glm::ivec2 calculate_gianttex_viewportoffsets(int matrixId);
	glm::ivec2 calculate_array_of_gianttex_offsets(int matrixId);
	uint32_t max_concurrent_viewports() const;
	uint32_t max_num_views_per_draw_call() const;

	bool activate_next_set() override;
	bool activate_prev_set() override;

	EvalConfig& get_eval_config() override { return m_eval_config; }
	TechniqueConfig& get_technique_config() override { return m_technique_config; }
	
//protected:
	std::array<std::string, 3> getShaders() override;
	TechniqueConfig m_technique_config;
	std::vector<std::vector<std::array<gamedevs::Plane, 6>>> m_frustum_planes;
	std::vector<std::vector<std::array<glm::vec4, 4>>> m_frustum_planes_left_right_top_bottom;
	std::string m_vert, m_geom, m_frag;
	size_t m_num_targets; //< To be set during init
	size_t m_views_per_target;
	bool m_has_Vs;
	int m_Vs_buffer;
	bool m_has_V;
	bool m_has_LAYER;
	bool m_has_uIdOffset;
	bool m_has_uLayerOffset;
	bool m_has_uVsOffset;
	bool m_has_NUM_PRIMITIVES;
	bool m_has_RESOLUTION;
	bool m_has_NUM_DRAWS_PER_VIEW;
	bool m_has_PLANE_CLIPPING;
	bool m_has_uNumTiles;
	GLuint m_CLIP_PLANES_buffer;
	std::optional<int> m_tmp_buffer_to_cleanup;
	std::optional<GLuint> m_primitive_mapping_offsets_buffer;
	size_t m_activeSet;
	EvalConfig m_eval_config;

	int currentlyActiveFramebuffer = -1;
};
