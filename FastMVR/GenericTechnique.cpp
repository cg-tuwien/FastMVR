#include "GenericTechnique.h"
#include <functional>
//#include <GL/glew.h>
#include <glad/glad.h>
#include "string_utils.h"
#include <glm/gtc/type_ptr.hpp>
#include <regex>
#include <stb_image_write.h>
#include "GLHelper.h"

GenericTechnique::GenericTechnique(std::string name, TechniqueConfig technCfg, EvalConfig evCfg, GeometryProviderBase& geometry, std::string vert, std::string geom, std::string frag, AdditionalShaderCode shaderAdd)
	: RenderPipeline(std::move(name), evCfg.resolution_x, evCfg.resolution_y, geometry)
	, m_technique_config{ technCfg }
	, m_eval_config{ evCfg }
	, m_vert{ insert_into_shader_code_after_version(std::move(vert), shaderAdd.m_additionToVertexShader) }
	, m_geom{ insert_into_shader_code_after_version(std::move(geom), shaderAdd.m_additionToGeometryShader) }
	, m_frag{ insert_into_shader_code_after_version(std::move(frag), shaderAdd.m_additionToFragementShader) }
	, m_num_targets{ 0 }
	, m_activeSet{0}
{
}

GenericTechnique::~GenericTechnique()
{
	if (m_CLIP_PLANES_buffer) {
		GL_CALL(glDeleteBuffers(1, &m_CLIP_PLANES_buffer));
	}
}

bool GenericTechnique::has_shader_uniform(std::string name, GLenum dataType)
{
	const auto loc = glGetUniformLocation(resources()->get_gl_handle(program_handle()), name.c_str());
	if (-1 == loc) return false;
	GLint size; 
	GLenum type;
	
	glGetActiveUniform(resources()->get_gl_handle(program_handle()), loc, 0, nullptr, &size, &type, nullptr);
	if (type != dataType) {
		std::cout << "\nWARNING: Variable '" << name << "' appears to have a different data type in shader\n" << std::endl;
	}
	return true;
}

bool GenericTechnique::has_shader_uniform_block(std::string name)
{
	const auto loc = glGetUniformBlockIndex(resources()->get_gl_handle(program_handle()), name.c_str());
	if (-1 == loc) return false;
	return true;
}

bool GenericTechnique::shader_source_contains(std::string name)
{
	if (m_vert.find(name) != std::string::npos) return true;
	if (m_vert.find(name) != std::string::npos) return true;
	if (m_vert.find(name) != std::string::npos) return true;
	return false;
}

void GenericTechnique::init(RenderResources* res)
{
	// Save pointer to the resources... because, why not?
	save_resources_ptr(res);
#if defined(SHADOW_MAPPING) && !defined(GBUFFER)
	if (!get_technique_config().requires_fragment_shader_for_depth_only()) {
		m_frag = "";
	}
#endif
#ifdef GBUFFER
	if (!m_frag.empty()) {
		m_frag = insert_into_shader_code_after_version(m_frag, "#define GBUFFER");
	}
	if (!m_geom.empty()) {
		m_geom = insert_into_shader_code_after_version(m_geom, "#define GBUFFER");
	}
	if (!m_vert.empty()) {
		m_vert = insert_into_shader_code_after_version(m_vert, "#define GBUFFER");
	}
#endif
	RenderPipeline::buildProgram();
	m_has_Vs = has_shader_uniform_block("Vs");
	if (m_has_Vs) {
		auto numMatrices = get_view_matrices_count();
		RenderResources::BufferConfig conf(sizeof(glm::mat4) * numMatrices, RenderResources::BufferConfig::Usage::STREAM_DRAW);
		m_Vs_buffer = resources()->buildBuffer(conf);
	}
	m_has_PLANE_CLIPPING = shader_source_contains("#define PLANE_CLIPPING");
	if (m_has_PLANE_CLIPPING) {
		for (const auto& set : m_eval_config.sets_of_matrices) {
			auto& curFrustPlanes = m_frustum_planes.emplace_back();
			auto& curFrustPlanesLRTB = m_frustum_planes_left_right_top_bottom.emplace_back();
			for (const auto& matrix : set) {
				curFrustPlanes.push_back(gamedevs::ExtractPlanesGL(matrix, true));
				curFrustPlanesLRTB.push_back({{
					{ curFrustPlanes.back()[0].a, curFrustPlanes.back()[0].b, curFrustPlanes.back()[0].c, curFrustPlanes.back()[0].d }, // left
					{ curFrustPlanes.back()[1].a, curFrustPlanes.back()[1].b, curFrustPlanes.back()[1].c, curFrustPlanes.back()[1].d }, // right
					{ curFrustPlanes.back()[2].a, curFrustPlanes.back()[2].b, curFrustPlanes.back()[2].c, curFrustPlanes.back()[2].d }, // top 
					{ curFrustPlanes.back()[3].a, curFrustPlanes.back()[3].b, curFrustPlanes.back()[3].c, curFrustPlanes.back()[3].d }  // bottom
					//{ 1.0f, 0.0f, 0.0f, 0.0f },
					//{ 0.0f, 1.0f, 0.0f, 0.0f },
				}});
			}
		}
		auto numMatrices = get_view_matrices_count();
		GL_CALL(glGenBuffers(1, &m_CLIP_PLANES_buffer));
	}
	m_has_uNumTiles = has_shader_uniform("uNumTiles", GL_INT);
	m_has_V = has_shader_uniform("V", GL_FLOAT_MAT4);
	m_has_LAYER = has_shader_uniform("LAYER", GL_INT);
	m_has_uIdOffset = has_shader_uniform("uIdOffset", GL_INT);
	m_has_uLayerOffset = has_shader_uniform("uLayerOffset", GL_INT);
	m_has_uVsOffset = has_shader_uniform("uVsOffset", GL_INT);
	m_has_NUM_PRIMITIVES = has_shader_uniform("NUM_PRIMITIVES", GL_INT);
	m_has_RESOLUTION = has_shader_uniform("RESOLUTION", GL_INT_VEC2);
	m_has_NUM_DRAWS_PER_VIEW = has_shader_uniform("NUM_DRAWS_PER_VIEW", GL_INT);
	
	// Config parameters for the targets:
	//  - Which dimensions shall one target have?
	//  - How many layers are there if we have array textures?
	//  - How many targets do we produce? Maybe as many as we have view matrices; or maybe only one? 
	m_num_targets = 0;

	// The next two functions are changed depending on the type of medium...
	std::function<int(RenderResources*)> build_color_buffer = [](RenderResources*) -> int{ throw std::logic_error("not set"); };
	std::function<int(RenderResources*)> build_depth_buffer = [](RenderResources*) -> int{ throw std::logic_error("not set"); };
#ifdef GBUFFER
	std::function<int(RenderResources*)> build_normals_buffer= [](RenderResources*) -> int { throw std::logic_error("not set"); };
	std::function<int(RenderResources*)> build_triangleid_buffer= [](RenderResources*) -> int { throw std::logic_error("not set"); };
#endif
	// ...whereas the build_framebuffer functionality is the same or all of them
	std::function<int(RenderResources*)> build_framebuffer = [](RenderResources* r) -> int { return r->buildFramebuffer({}); };
	// How to attach the different textures/renderbuffers to the framebuffer also depend on the concrete settings => store behavior in function:
	// Parameters: 0th: u know, 1st: framebuffer-handle, 2nd: color-attachment handles, 3rd: depth attachment handle
	std::function<void(RenderResources*, int, const std::vector<int>&, int)> attach_to_framebuffer = [](RenderResources*, int, const std::vector<int>&, int) -> void{ throw std::logic_error("not set"); };
	// Determine the build_texture and build_renderbuffer functionality based on the type of medium
	switch (m_technique_config.m_type_of_medium)
	{
	case ViewTechnique::MultiViewMedium::SEPARATE_TEXTURES:
		// Alloc only one texture/renderbuffer:
		build_color_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), 1u /* layer */, RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildTexture(cfg); };
#ifdef SHADOW_MAPPING
		build_depth_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), 1u, RenderResources::TextureFormat::DEPTH }]
								(RenderResources* r) -> int{ return r->buildTexture(cfg); };
#else
		build_depth_buffer =	[cfg = RenderResources::RenderbufferConfig{ width(), height() }]
								(RenderResources* r) -> int{ return r->buildRenderbuffer(cfg); };
#endif
		attach_to_framebuffer =	[](RenderResources* r, int fb, const std::vector<int>& cols, int db) 
								{ r->combine(fb, cols, db); };
		
#ifdef GBUFFER
		build_normals_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), 1u /* layer */, RenderResources::TextureFormat::RGBA8 }]
								(RenderResources* r) -> int { return r->buildTexture(cfg); };
		build_triangleid_buffer=[cfg = RenderResources::ArrayTextureConfig{ width(), height(), 1u /* layer */, RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildTexture(cfg); };
#endif
		
		m_num_targets = get_view_matrices_count();
		m_views_per_target = 1;
		break;
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY:
		// Alloc an array of textures/renderbuffers:
		{
			GLint maxTexLayers;
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexLayers);
			assert (maxTexLayers >= get_view_matrices_count());
		}
		build_color_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(get_view_matrices_count()), RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
		build_depth_buffer=		[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(get_view_matrices_count()), RenderResources::TextureFormat::DEPTH }]
								(RenderResources* r) -> int{ return r->buildArrayTexture(cfg); };
		attach_to_framebuffer =	[](RenderResources* r, int fb, const std::vector<int>& cols, int db) 
								{ r->combineLayered(fb, cols, db); };
		
#ifdef GBUFFER
		build_normals_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(get_view_matrices_count()), RenderResources::TextureFormat::RGBA8 }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
		build_triangleid_buffer=[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(get_view_matrices_count()), RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
#endif

		if (ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW == m_technique_config.m_duplication_strategy) {
			assert(false); // => must use MULTIPLE_TEXTURE_ARRAYS!!
		}
		
		m_num_targets = 1; 
		m_views_per_target = get_view_matrices_count();
		break;
	case ViewTechnique::MultiViewMedium::MULTIPLE_TEXTURE_ARRAYS:
		assert(ViewTechnique::MultiViewChange::FRAMEBUFFER_THEN_LAYER == m_technique_config.m_view_change_strategy);
		
		// Alloc an array of textures/renderbuffers:
		{
			GLint maxTexLayers;
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTexLayers);
			assert (maxTexLayers >= max_num_views_per_draw_call());
		}
		build_color_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(max_num_views_per_draw_call()), RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
		build_depth_buffer=		[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(max_num_views_per_draw_call()), RenderResources::TextureFormat::DEPTH }]
								(RenderResources* r) -> int{ return r->buildArrayTexture(cfg); };
		attach_to_framebuffer =	[](RenderResources* r, int fb, const std::vector<int>& cols, int db) 
								{ r->combineLayered(fb, cols, db); };
		
#ifdef GBUFFER
		build_normals_buffer =	[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(max_num_views_per_draw_call()), RenderResources::TextureFormat::RGBA8 }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
		build_triangleid_buffer=[cfg = RenderResources::ArrayTextureConfig{ width(), height(), static_cast<uint32_t>(max_num_views_per_draw_call()), RenderResources::TextureFormat::R32UI }]
								(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
#endif

		if (ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW == m_technique_config.m_duplication_strategy) {
			// If we are using NV's/OVR's Multi-View extension, we need to build the framebuffer with targets attached
			// in a special way; hence, set those functions accordingly:
			attach_to_framebuffer =	[num_ovr_views = static_cast<int>(max_num_views_per_draw_call())]
									(RenderResources* r, int fb, const std::vector<int>& cols, int db)
									{ r->combineLayeredMultiview(fb, cols, db, num_ovr_views); };
		}
		
		m_num_targets = get_view_matrices_count() / max_num_views_per_draw_call();
		if (get_view_matrices_count() % max_num_views_per_draw_call() != 0) { m_num_targets += 1; }
		m_views_per_target = max_num_views_per_draw_call();
		break;
	case ViewTechnique::MultiViewMedium::GIANT_TEXTURE:
	case ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES:
		// Alloc only one... BUT GIANT texture/renderbuffer:
		assert (
			(ViewTechnique::MultiViewMedium::GIANT_TEXTURE == m_technique_config.m_type_of_medium && (ViewTechnique::MultiViewChange::VIEWPORT == m_technique_config.m_view_change_strategy	|| ViewTechnique::MultiViewChange::NONE		== m_technique_config.m_view_change_strategy))
			|| (ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES == m_technique_config.m_type_of_medium && (ViewTechnique::MultiViewChange::FRAMEBUFFER_THEN_VIEWPORT == m_technique_config.m_view_change_strategy || ViewTechnique::MultiViewChange::FRAMEBUFFER_THEN_NONE == m_technique_config.m_view_change_strategy))
		);
		{
			auto numPerDrawcall = get_view_matrices_count();
			if (m_technique_config.m_max_num_gtex_tiles.has_value()) {
				numPerDrawcall = std::min(numPerDrawcall, m_technique_config.m_max_num_gtex_tiles.value());
			}
			const auto numPerRow = static_cast<uint32_t>(std::ceil(std::sqrt(static_cast<double>(numPerDrawcall))));
			const auto numPerCol = numPerDrawcall % numPerRow == 0 ? numPerDrawcall / numPerRow : numPerDrawcall / numPerRow + 1;
			const auto giantResolutionWidth = width() * numPerRow;
			const auto giantResulutionHeight = height() * numPerCol;
			build_color_buffer =	[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, 1u /* layer */, RenderResources::TextureFormat::R32UI }]
									(RenderResources* r) -> int { return r->buildTexture(cfg); };
#ifdef SHADOW_MAPPING
			build_depth_buffer=		[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, 1u, RenderResources::TextureFormat::DEPTH }]
									(RenderResources* r) -> int { return r->buildTexture(cfg); };
#else
			build_depth_buffer=		[cfg = RenderResources::RenderbufferConfig{  giantResolutionWidth, giantResulutionHeight }]
									(RenderResources* r) -> int{ return r->buildRenderbuffer(cfg); };
#endif
			attach_to_framebuffer =	[](RenderResources* r, int fb, const std::vector<int>& cols, int db) 
									{ r->combine(fb, cols, db); };
		
#ifdef GBUFFER
		build_normals_buffer =		[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, 1u /* layer */, RenderResources::TextureFormat::RGBA8 }]
									(RenderResources* r) -> int { return r->buildTexture(cfg); };
		build_triangleid_buffer =	[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, 1u /* layer */, RenderResources::TextureFormat::R32UI }]
									(RenderResources* r) -> int { return r->buildTexture(cfg); };
#endif

			m_num_targets = get_view_matrices_count() / numPerDrawcall;
			if (get_view_matrices_count() % numPerDrawcall != 0) { m_num_targets += 1; }
			m_views_per_target = numPerDrawcall;
		}
		break;
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES:
		assert(ViewTechnique::MultiViewChange::LAYER_THEN_NONE == m_technique_config.m_view_change_strategy || ViewTechnique::MultiViewChange::LAYER_THEN_VIEWPORT == m_technique_config.m_view_change_strategy);
		{
			const auto numPerLayer = m_technique_config.m_max_num_gtex_tiles.value_or(1);
			if (1 == numPerLayer) {
				std::cout << "\nWARNING: Using TEXTURE_ARRAY_OF_GIANT_TEXTURES, but there is only one tile per layer.\n" << std::endl;
			}
			const auto numPerRow = static_cast<uint32_t>(std::ceil(std::sqrt(static_cast<double>(numPerLayer))));
			const auto numPerCol = numPerLayer % numPerRow == 0 ? numPerLayer / numPerRow : numPerLayer / numPerRow + 1;
			const auto giantResolutionWidth = width() * numPerRow;
			const auto giantResulutionHeight = height() * numPerCol;
		
			assert(get_view_matrices_count() == max_num_views_per_draw_call());
			auto numLayers = max_num_views_per_draw_call() / numPerLayer;
			if (max_num_views_per_draw_call() % numPerLayer != 0) { numLayers += 1; }
		
			build_color_buffer =	[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, static_cast<uint32_t>(numLayers), RenderResources::TextureFormat::R32UI }]
									(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
			build_depth_buffer=		[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, static_cast<uint32_t>(numLayers), RenderResources::TextureFormat::DEPTH }]
									(RenderResources* r) -> int{ return r->buildArrayTexture(cfg); };
			attach_to_framebuffer =	[](RenderResources* r, int fb, const std::vector<int>& cols, int db) 
									{ r->combineLayered(fb, cols, db); };
		
#ifdef GBUFFER
		build_normals_buffer =		[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, static_cast<uint32_t>(numLayers), RenderResources::TextureFormat::RGBA8 }]
									(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
		build_triangleid_buffer =	[cfg = RenderResources::ArrayTextureConfig{ giantResolutionWidth, giantResulutionHeight, static_cast<uint32_t>(numLayers), RenderResources::TextureFormat::R32UI }]
									(RenderResources* r) -> int { return r->buildArrayTexture(cfg); };
#endif

			if (ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW == m_technique_config.m_duplication_strategy) {
				// If we are using NV's/OVR's Multi-View extension, we need to build the framebuffer with targets attached
				// in a special way; hence, set those functions accordingly:
				attach_to_framebuffer =	[num_ovr_views = numLayers]
										(RenderResources* r, int fb, const std::vector<int>& cols, int db)
										{ r->combineLayeredMultiview(fb, cols, db, num_ovr_views); };
			}
					
			m_num_targets = 1;
			m_views_per_target = numPerLayer * numLayers;
			assert(get_view_matrices_count() <= m_views_per_target);
		}
		break;
	default:
		assert(false); throw std::domain_error("wtf?!");
	}
	
	// Build textures, renderbuffers, framebuffers based on the strategy.
	// Also use the std::functions determined above during this step.
	assert(m_num_targets > 0);
	for (size_t i = 0; i < m_num_targets; ++i)
	{
		// Create new stuff on the GPU...
		const auto colorBufferHandle = textures_for_range(0).emplace_back(build_color_buffer(resources()));
		const auto depthBufferHandle = renderbuffers_for_range(0).emplace_back(build_depth_buffer(resources()));
		const auto framebufferHandle = framebuffers_for_range(0).emplace_back(build_framebuffer(resources()));
		// ... and tie them together
#ifdef GBUFFER
		const auto normalsBufferHandle = textures_for_range(1).emplace_back(build_normals_buffer(resources()));
		const auto triangleidBufferHandle = textures_for_range(2).emplace_back(build_triangleid_buffer(resources()));
		attach_to_framebuffer(resources(), framebufferHandle, { colorBufferHandle, normalsBufferHandle, triangleidBufferHandle }, depthBufferHandle);
#else
		attach_to_framebuffer(resources(), framebufferHandle, { colorBufferHandle }, depthBufferHandle);
#endif
		// ... and enable the framebuffer (whatever that means... it calls glDrawBuffers internally... whatever that does) => Update: Enables multiple color-outputs (used for GBUFFER)
#ifdef GBUFFER
		resources()->enableTargets(framebufferHandle, { 0, 1, 2 });
#else
		resources()->enableTargets(framebufferHandle, { 0 });
#endif
		assert(resources()->checkFramebuffer(framebufferHandle));
	}

	// We might need a buffer for storing multiple primitive-id mapping offsets:
	if (geometry_provider().requiresPrimitiveIdMapping() && ViewTechnique::MultiViewDuplication::MULTIDRAW_INDIRECT == m_technique_config.m_duplication_strategy) {
		GLuint handle;
		GL_CALL(glCreateBuffers(1, &handle));
		std::vector<uint32_t> tmp(geometry_provider().maxNumberOfPrimitiveMappingOffsets(), 0u);
		auto bufferSize = sizeof(tmp[0]) * geometry_provider().maxNumberOfPrimitiveMappingOffsets();
		m_primitive_mapping_offsets_buffer = handle;
		GL_CALL(
			glNamedBufferData(m_primitive_mapping_offsets_buffer.value(), bufferSize, tmp.data(), GL_DYNAMIC_DRAW)
		);
	}
}

void GenericTechnique::begin() 
{
	if (m_has_PLANE_CLIPPING) {
		glEnable(GL_CLIP_DISTANCE0);
		glEnable(GL_CLIP_DISTANCE1);
		glEnable(GL_CLIP_DISTANCE2);
		glEnable(GL_CLIP_DISTANCE3);
	}

	if (m_has_uNumTiles) {
		const int numTiles = m_technique_config.m_max_num_gtex_tiles.value_or(1);
		resources()->setUniform(RenderResources::UniformType::INT, "uNumTiles", &numTiles);
	}
	
	// Needs baked, has no concept of matrices, hence "true"
	// Q: Is this true for all the techniques?
	if (ViewTechnique::MultiViewDuplication::GIANT_INDEX_BUFFER == m_technique_config.m_duplication_strategy) {
		// Creates a blown-up index buffer which simply multiplies the amount of geometry that is submitted
		resources()->begin(program_handle(), get_view_matrices_count(), m_technique_config.m_index_buffer_duplication_strategy.value_or(duplication_strategy::consecutive));
	}
	else {
		resources()->begin(program_handle());
	}

	if (m_has_uIdOffset) {
		const int idOffset = 0;
		resources()->setUniform(RenderResources::UniformType::INT, "uIdOffset", &idOffset);
	}

	if (m_has_uLayerOffset) {
		const int layerOffset = 0;
		resources()->setUniform(RenderResources::UniformType::INT, "uLayerOffset", &layerOffset);
	}

	if (m_has_uVsOffset) {
		const int vOffset = 0;
		resources()->setUniform(RenderResources::UniformType::INT, "uVsOffset", &vOffset);
	}
	
	if (m_has_NUM_DRAWS_PER_VIEW) {
		auto numDrawsPerView = 1u;
		resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);
	}

	if (m_has_Vs) {
		resources()->bufferData(m_Vs_buffer, 0, get_view_proj_matrices_data_ptr(), sizeof(glm::mat4) * get_view_matrices_count());
		resources()->bindUniformBufferRange(m_Vs_buffer, "Vs", sizeof(glm::mat4) * get_view_matrices_count());
	}

	if (m_has_PLANE_CLIPPING) {
		auto numMatrices = get_view_matrices_count();
		GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CLIP_PLANES_buffer));
		GL_CALL(
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * numMatrices * 4, m_frustum_planes_left_right_top_bottom[m_activeSet].data(), GL_DYNAMIC_DRAW)
		);
		GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
		GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_CLIP_PLANES_buffer));  // layout(std140, binding = 5) buffer CLIP_PLANES
	}

	// Clear all targets upfront:
	for (size_t i = 0; i < m_num_targets; ++i) {
		resources()->activateFramebuffer(framebuffers_for_range(0)[i]);
		resources()->clear();
	}
	
	if (m_num_targets == 1) {
		auto numMatrices = get_view_matrices_count();

		// Update uniform buffer with view matrix data.
		switch (m_technique_config.m_duplication_strategy)
		{
		case ViewTechnique::MultiViewDuplication::MULTIDRAW_INDIRECT:
			if (m_technique_config.m_max_num_views_to_produce_with_one_draw_call.has_value()) {
				std::cout << "WARNING: limited number of concurrent views not yet supported for MULTIDRAW_INDIRECT" << std::endl;
			}
			{
				assert(m_has_NUM_DRAWS_PER_VIEW);
				auto geometries = geometry_provider().getGeometryForMatrices(get_all_view_proj_matrices());
				auto numDrawsPerView = static_cast<int>(geometries.size());
				resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);
				if (geometries.size() > 0) 
				{
					if (geometry_provider().requiresPrimitiveIdMapping()) 
					{
						
						assert(geometries.front().m_primitive_id_mapping_buffer != 0);
						GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, geometries.front().m_primitive_id_mapping_buffer));  // layout(std430, binding = 0) buffer PrimitiveIdMapping
						
						// For each geometry, gather their offsets into the primitive-mapping buffers into the following vector and upload it to the GPU afterwards:
						std::vector<uint32_t> primitiveMappingOffsets;
						primitiveMappingOffsets.reserve(numDrawsPerView);
						for (int g = 0; g < numDrawsPerView; ++g)
						{
							primitiveMappingOffsets.push_back(geometries[g].m_offset_into_primitive_id_mapping_buffer);
						}
						assert(m_primitive_mapping_offsets_buffer.has_value());
						auto bufferHandle = m_primitive_mapping_offsets_buffer.value();
						auto dataSize = sizeof(uint32_t) * primitiveMappingOffsets.size();
						//GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferHandle));
						/*GL_CALL(
							glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dataSize, primitiveMappingOffsets.data())
						);*/
						//GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, primitiveMappingOffsets.data(), GL_DYNAMIC_DRAW));
						//GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
						GL_CALL(
							glNamedBufferSubData(m_primitive_mapping_offsets_buffer.value(), 0, dataSize, primitiveMappingOffsets.data())
						);
						GL_CALL(
							glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_primitive_mapping_offsets_buffer.value()) // layout(std430, binding = 1) buffer PrimitiveIdMappingOffsets
						);
					}

					std::vector<RenderResources::DrawElementsIndirectCommand> indirects;
					indirects.reserve(numMatrices * numDrawsPerView);
					for (int i = 0; i < numMatrices; i++) // for each matrix...
					{
						for (int g = 0; g < numDrawsPerView; ++g) // all draws PER MATRIX
						{
							auto drawcallData = geometries[g].m_drawcall_data;
							drawcallData.firstIndex = drawcallData.firstIndex / sizeof(uint32_t); // What the fucking fuck? Why has anyone started using OpenGL at all? This API is hell.
							indirects.push_back(drawcallData);
						}
					}
					assert(indirects.size() == numDrawsPerView * numMatrices);
					int buff = resources()->buildIndirectsBuffer(RenderResources::BufferConfig::Usage::STATIC_DRAW, indirects);
					resources()->setIndirect(buff);
					m_tmp_buffer_to_cleanup = buff;
				}
			} 
			break; // end of case ViewTechnique::MultiViewDuplication::MULTIDRAW_INDIRECT

		}// end of switch (m_technique_config.m_duplication_strategy)

		// We only have one framebuffer, let's activate it before rendering.
		resources()->activateFramebuffer(framebuffers_for_range(0)[0]);
	}

	// Set a Viewport if config is not MULTIDRAW+VIEWPORT
	if (ViewTechnique::MultiViewDuplication::MULTIDRAW != m_technique_config.m_duplication_strategy && ViewTechnique::MultiViewChange::VIEWPORT != m_technique_config.m_view_change_strategy) 
	{
		auto config = resources()->getTextureConfig(textures_for_range(0)[0]);
		resources()->activateViewport(RenderResources::ViewportConfig{0, 0, static_cast<float>(config.width), static_cast<float>(config.height)});
	}
}

void GenericTechnique::produceLevel(uint32_t level, ChangeT viewChangeType, MediumT viewMediumType, uint32_t viewStartIndex, const std::vector<glm::mat4>& mats)
{
	switch (viewChangeType) {
	case ChangeT::FRAMEBUFFER_THEN_NONE:
		assert(level < 2);
		assert(MediumT::MULTIPLE_GIANT_TEXTURES == viewMediumType);
		produceLevel(level + 1, ChangeT::NONE, MediumT::GIANT_TEXTURE, viewStartIndex, mats);
		return;
	case ChangeT::FRAMEBUFFER_THEN_VIEWPORT:
		assert(level < 2);
		assert(MediumT::MULTIPLE_GIANT_TEXTURES == viewMediumType);
		for (uint32_t i = 0; i < mats.size(); ++i) {
			produceLevel(level + 1, ChangeT::VIEWPORT, MediumT::GIANT_TEXTURE, viewStartIndex + i, { mats[i] });
		}
		return;
	case ChangeT::LAYER_THEN_NONE:
		assert(level < 2);
		assert(MediumT::TEXTURE_ARRAY_OF_GIANT_TEXTURES == viewMediumType);
		produceLevel(level + 1, ChangeT::NONE, MediumT::GIANT_TEXTURE, viewStartIndex, mats);
		return;
	case ChangeT::LAYER_THEN_VIEWPORT:
		assert(level < 2);
		assert(MediumT::TEXTURE_ARRAY_OF_GIANT_TEXTURES == viewMediumType);
		// TODO: Loop or what?
		produceLevel(level + 1, ChangeT::VIEWPORT, MediumT::GIANT_TEXTURE, viewStartIndex, mats);
		return;
	case ChangeT::FRAMEBUFFER_THEN_LAYER:
		assert(level < 2);
		assert(MediumT::MULTIPLE_TEXTURE_ARRAYS == viewMediumType);
		produceLevel(level + 1, ChangeT::LAYER, MediumT::TEXTURE_ARRAY, viewStartIndex, mats);
		return;
	}

	if (m_has_V) {
		assert(1 == mats.size());
		resources()->setUniform(RenderResources::UniformType::MATRIX4x4, "V", glm::value_ptr(mats[0]));
	}

	if (m_has_LAYER) {
		assert(1 == mats.size());
		resources()->setUniform(RenderResources::UniformType::INT, "LAYER", &viewStartIndex); 
	}

	// Possibly set a a framebuffer:
	if (m_num_targets > 1) {
		//assert (1 == mats.size() || mats.size() == max_num_views_per_draw_call());
		const auto framebufferId = viewStartIndex / max_num_views_per_draw_call();
		const auto framebufferToBeActivated = framebuffers_for_range(0)[framebufferId];
		if (currentlyActiveFramebuffer != framebufferToBeActivated) {
			resources()->activateFramebuffer(framebufferToBeActivated);
			currentlyActiveFramebuffer = framebufferToBeActivated;		
		}
		// else: already activated and cleared in begin()
	}

	if (m_has_uVsOffset) {
		const int offsetIntoVs = viewStartIndex;
		resources()->setUniform(RenderResources::UniformType::INT, "uVsOffset", &offsetIntoVs);
	}
	
	// Possibly set the viewport to somewhere in the giant texture:
	if (MediumT::GIANT_TEXTURE == viewMediumType && ChangeT::VIEWPORT == viewChangeType) {
		std::vector<RenderResources::ViewportConfig> viewports;
		for (int i = 0; i < mats.size(); ++i) {
			viewports.push_back(calculate_gianttex_viewportconfig(viewStartIndex + i)); // offset by viewStartIndex!
		}
		assert(mats.size() == viewports.size());
		resources()->activateViewports(viewports); // i, not b, because of MULTIPLE_GIANT_TEXTURES
	}

	switch (m_technique_config.m_duplication_strategy)
	{
	case ViewTechnique::MultiViewDuplication::MULTIPASS:
	case ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW:
		if (ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES == m_technique_config.m_type_of_medium && ViewTechnique::MultiViewChange::LAYER_THEN_NONE == m_technique_config.m_view_change_strategy) {
			// TODO ^: This is ugly!
			// TODO v: Not sure if m_technique_config.m_max_num_gtex_tiles.value() is the right choice.
			resources()->renderAllInstanced(m_technique_config.m_max_num_gtex_tiles.value(), geometry_provider(), mats);
		}
		else {
			resources()->renderAll(geometry_provider(), mats);
		}
		break;
	case ViewTechnique::MultiViewDuplication::MULTIDRAW:
		resources()->renderAllMulti(mats.size(), geometry_provider(), mats);
		break;
	case ViewTechnique::MultiViewDuplication::MULTIDRAW_INDIRECT:
		assert(mats.size() == get_all_view_proj_matrices().size());
		{
			auto geometries = geometry_provider().getGeometryForMatrices(mats);
			auto numDrawsPerView = static_cast<int>(geometries.size());
			if (geometries.size() > 0) 
			{
				const auto& first = geometries.front();
#if defined(_DEBUG)
				for (auto& r : geometries) 
				{
					// This method can only be used with GeometryProviders that store everything in ONE big VAO/vertex buffer/index buffer
					assert(first.m_vao == r.m_vao);
					assert(first.m_vertex_buffer == r.m_vertex_buffer);
					assert(first.m_index_buffer == r.m_index_buffer);
				}
#endif
				resources()->renderIndirectMulti(numDrawsPerView * mats.size(), first);	
			}
		}
		break;
	case ViewTechnique::MultiViewDuplication::GEOMETRY_SHADER_LOOP:
	case ViewTechnique::MultiViewDuplication::GEOMETRY_SHADER_INSTANCING:
	case ViewTechnique::MultiViewDuplication::GIANT_INDEX_BUFFER:
		resources()->renderAll(geometry_provider(), mats);
		break;
	case ViewTechnique::MultiViewDuplication::INSTANCING:
		resources()->renderAllInstanced(mats.size(), geometry_provider(), mats);
		break;
	default: ;
		assert(false); throw std::domain_error("wtf?!");
	}
}

void GenericTechnique::produce() 
{
	auto numMatrices = get_view_matrices_count();

	if (m_has_NUM_PRIMITIVES) {
		std::cout << "\nWARNING: NUM_PRIMITIVES variable not supported right now. Was used by the mediocre index-buffer-duplication techniques, anyways.\n" << std::endl;
		//auto numPrim = resources()->totalIndices();
		//resources()->setUniform(RenderResources::UniformType::INT, "NUM_PRIMITIVES", &numPrim);
	}

	if (m_has_RESOLUTION) {
		glm::ivec2 resolution(width(), height());
		resources()->setUniform(RenderResources::UniformType::IVEC2, "RESOLUTION", &resolution.x);
		//// The following would be texture size:
		//auto config = resources()->getTextureConfig(textures_for_range(0)[0]);
		//glm::ivec2 resolution{config.width, config.height};
		//resources()->setUniform(RenderResources::UniformType::IVEC2, "RESOLUTION", &resolution.x);
	}
	
	{
		auto M = max_num_views_per_draw_call(); // TODO: Also consider m_technique_config.m_max_num_gtex_tiles.value()?
		auto N = numMatrices;
		for (decltype(M) b = 0; b < N; b += M) {
			auto remaining = N - b;
			auto mCount = std::min(remaining, M);
			
			auto mats = get_view_proj_matrices_range(b, b + mCount);
			assert (mats.size() == mCount);

			if (m_has_NUM_DRAWS_PER_VIEW) {
				auto geometries = geometry_provider().getGeometryForMatrices(mats);
				auto numDrawsPerView = static_cast<int>(geometries.size());
				resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);

				if (ViewTechnique::MultiViewDuplication::MULTIDRAW == m_technique_config.m_duplication_strategy) { // TODO: WHYYYYYY?
					numDrawsPerView = 1;
					resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);
				}
			}

			if (m_has_uIdOffset) {
				const int idOffset = b;
				resources()->setUniform(RenderResources::UniformType::INT, "uIdOffset", &idOffset);
			}

			if (m_has_uLayerOffset) {
				int layerOffset = b; // same as uIdOffset by default, except...
				if (ViewTechnique::MultiViewMedium::MULTIPLE_TEXTURE_ARRAYS == m_technique_config.m_type_of_medium) {
					layerOffset = 0;
				}
				resources()->setUniform(RenderResources::UniformType::INT, "uLayerOffset", &layerOffset);
			}

			produceLevel(0, m_technique_config.m_view_change_strategy, m_technique_config.m_type_of_medium, b, mats);
		}
	}

//	switch (m_technique_config.m_duplication_strategy)
//	{
//	case ViewTechnique::MultiViewDuplication::MULTIPASS:
//		// Do something for each view position (for each view matrix):
//		for (size_t i = 0; i < numMatrices; ++i) 
//		{
//			if (m_has_V) {
//				resources()->setUniform(RenderResources::UniformType::MATRIX4x4, "V", glm::value_ptr(get_view_proj_matrix(i)));
//			}
//
//			if (m_has_LAYER) {
//				resources()->setUniform(RenderResources::UniformType::INT, "LAYER", &i);
//			}
//
//			// Possibly set a a framebuffer:
//			if (m_num_targets > 1) {
//				assert(m_num_targets == numMatrices);
//				resources()->activateFramebuffer(framebuffers_for_range(0)[i]);
//				resources()->clear();
//				// else: already activated and cleared in begin()
//			} 
//			
//			// Possibly set the viewport to somewhere in the giant texture:
//			if (ViewTechnique::MultiViewMedium::GIANT_TEXTURE == m_technique_config.m_type_of_medium) {
//				resources()->activateViewports({ calculate_gianttex_viewportconfig(i) });
//			}
//
//			// Clear and go:
//			resources()->renderAll(geometry_provider(), { get_view_proj_matrix(i) }); // <-- ATTENTION! Only one matrix here!
//		}
//		break;
//	case ViewTechnique::MultiViewDuplication::MULTIDRAW:
//		if (m_technique_config.m_max_concurrent_views.has_value()) {
//			std::cout << "WARNING: limited number of concurrent views not yet supported for MULTIDRAW" << std::endl;
//		}
//		resources()->clear();
//		if (ViewTechnique::MultiViewChange::VIEWPORT == m_technique_config.m_view_change_strategy) 
//		{
//			auto maxViewports = max_concurrent_viewports();
//			int matrixId = 0;
//			for (decltype(maxViewports) v = 0; v < static_cast<uint32_t>(numMatrices); v += maxViewports) 
//			{
//				auto remainder = numMatrices - v;
//				auto n = std::min(remainder, maxViewports);
//				
//				std::vector<RenderResources::ViewportConfig> viewports;
//				for (decltype(n) i = 0; i < n; i++)
//				{
//					viewports.push_back(calculate_gianttex_viewportconfig(matrixId++));
//				}
//
//				auto mats = get_view_proj_matrices_range(v, matrixId);
//				if (m_has_NUM_DRAWS_PER_VIEW) {
//					//auto geometries = geometry_provider().getGeometryForMatrices(mats);
//					//auto numDrawsPerView = static_cast<int>(geometries.size());
//					auto numDrawsPerView = 1;
//					resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);
//				}
//
//				if (m_has_uVsOffset) {
//					const int offsetIntoVs = v;
//					resources()->setUniform(RenderResources::UniformType::INT, "uVsOffset", &offsetIntoVs);
//				}
//				
//				resources()->activateViewports(viewports);
//				resources()->renderAllMulti(mats.size(), geometry_provider(), mats);	
//			}
//		}
//		else 
//		{
//			resources()->renderAllMulti(numMatrices, geometry_provider(), get_all_view_proj_matrices());	
//		}
//		break;
//	case ViewTechnique::MultiViewDuplication::MULTIDRAW_INDIRECT:
//		{
//			resources()->clear();
//			
//			auto geometries = geometry_provider().getGeometryForMatrices(get_all_view_proj_matrices());
//			auto numDrawsPerView = static_cast<int>(geometries.size());
//			if (geometries.size() > 0) 
//			{
//				const auto& first = geometries.front();
//#if defined(_DEBUG)
//				for (auto& r : geometries) 
//				{
//					// This method can only be used with GeometryProviders that store everything in ONE big VAO/vertex buffer/index buffer
//					assert(first.m_vao == r.m_vao);
//					assert(first.m_vertex_buffer == r.m_vertex_buffer);
//					assert(first.m_index_buffer == r.m_index_buffer);
//				}
//#endif
//				resources()->renderIndirectMulti(numDrawsPerView * numMatrices, first);	
//			}
//		}
//		break;
//	case ViewTechnique::MultiViewDuplication::GEOMETRY_SHADER_LOOP:
//	case ViewTechnique::MultiViewDuplication::GEOMETRY_SHADER_INSTANCING:
//	case ViewTechnique::MultiViewDuplication::GIANT_INDEX_BUFFER:
//		if (m_technique_config.m_max_concurrent_views.has_value()) {
//			std::cout << "WARNING: limited number of concurrent views not yet supported for GEOMETRY_SHADER_LOOP|GEOMETRY_SHADER_INSTANCING|GIANT_INDEX_BUFFER" << std::endl;
//		}
//		resources()->clear();
//		resources()->renderAll(geometry_provider(), get_all_view_proj_matrices());
//		break;
//	case ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW:
//		{
//			auto M = max_num_views_per_draw_call();
//			auto N = numMatrices;
//			int fb = 0;
//			for (decltype(M) b = 0; b < N; b += M) {
//				auto remaining = N - b;
//				auto mCount = std::min(remaining, M);
//				
//				auto mats = get_view_proj_matrices_range(b, b + mCount);
//				assert (mats.size() == mCount);
//
//				if (m_has_NUM_DRAWS_PER_VIEW) {
//					auto geometries = geometry_provider().getGeometryForMatrices(mats);
//					auto numDrawsPerView = static_cast<int>(geometries.size());
//					resources()->setUniform(RenderResources::UniformType::INT, "NUM_DRAWS_PER_VIEW", &numDrawsPerView);
//				}
//
//				if (m_has_uIdOffset) {
//					const int idOffset = b;
//					resources()->setUniform(RenderResources::UniformType::INT, "uIdOffset", &idOffset);
//				}
//
//				resources()->activateFramebuffer(framebuffers_for_range(0)[fb++]);
//				resources()->clear();
//				if (ViewTechnique::MultiViewMedium::ARRAY_OF_GIANT_TEXTURES == m_technique_config.m_type_of_medium) {
//					resources()->renderAllInstanced(mCount / m_technique_config.m_max_concurrent_views.value(), geometry_provider(), mats); // TODO: This can't be right => proceed here with ovr_x4_gtex-adaption!
//				}
//				else {
//					resources()->renderAll(geometry_provider(), mats);
//				}
//			}
//		}
//		break;
//	case ViewTechnique::MultiViewDuplication::INSTANCING:
//		if (m_technique_config.m_max_concurrent_views.has_value()) {
//			std::cout << "WARNING: limited number of concurrent views not yet supported for INSTANCING" << std::endl;
//		}
//		resources()->clear();
//		resources()->renderAllInstanced(numMatrices, geometry_provider(), get_all_view_proj_matrices());
//		break;
//	default: ;
//		assert(false); throw std::domain_error("wtf?!");
//	}
}

void GenericTechnique::end()
{
	if (m_tmp_buffer_to_cleanup.has_value()) {
		resources()->deleteBuffer(m_tmp_buffer_to_cleanup.value());
		m_tmp_buffer_to_cleanup = {};
	}

	if (m_has_PLANE_CLIPPING) {
		glDisable(GL_CLIP_DISTANCE0);
		glDisable(GL_CLIP_DISTANCE1);
		glDisable(GL_CLIP_DISTANCE2);
		glDisable(GL_CLIP_DISTANCE3);
	}
}

void GenericTechnique::dumpImages(const char* directory)
{
	auto config = resources()->getTextureConfig(textures_for_range(0)[0]);
	uint32_t w = config.width;
	uint32_t h = config.height;
	std::vector<uint32_t> data(w * h);

	for (int trg = 0; trg < m_num_targets; trg++)
	{
		if (ViewTechnique::MultiViewMedium::TEXTURE_ARRAY == m_technique_config.m_type_of_medium 
			|| ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES == m_technique_config.m_type_of_medium
			|| ViewTechnique::MultiViewMedium::MULTIPLE_TEXTURE_ARRAYS == m_technique_config.m_type_of_medium) 
		{
			config = resources()->getTextureConfig(textures_for_range(0)[trg]);
			for (int lyr = 0; lyr < config.num_layers; ++lyr) 
			{
				const int texIndex = lyr / m_views_per_target;
				const int layerIndex = lyr - (texIndex * m_views_per_target);
				
				resources()->fetchTextureLayer(textures_for_range(0)[trg], layerIndex, data);
				for (int i = 0; i < data.size(); i++)
				{
					data[i] |= 0xFF000000; // set full alpha. hack! remove later!
				}
				std::string name = std::string(directory) + std::string("v") + std::to_string(trg * m_views_per_target + lyr) + std::string("_d") + std::to_string(0) + std::string(".png");
				stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
			}
		}
		else 
		{
			resources()->fetchTextureLayer(textures_for_range(0)[trg], 0, data);
			for (int i = 0; i < data.size(); i++)
			{
				data[i] |= 0xFF000000; // set full alpha. hack! remove later!
			}
			std::string name = std::string(directory) + std::string("v") + std::to_string(trg) + std::string("_d") + std::to_string(0) + std::string(".png");
			stbi_write_png(name.c_str(), w, h, 4, data.data(), w * 4);
			
		}

		if (ViewTechnique::MultiViewMedium::GIANT_TEXTURE == m_technique_config.m_type_of_medium) 
		{
			break; // TODO: Do we really always have only one giant texture?
		}
	}
}

int GenericTechnique::get_view_matrices_count() const
{
	return static_cast<int>(m_eval_config.number_of_matrices_per_view_position());
}

glm::mat4 GenericTechnique::get_view_proj_matrix(int i)
{
	return m_eval_config.sets_of_matrices[m_activeSet][i];
}

glm::mat4 GenericTechnique::get_view_matrix(int i)
{
	return glm::inverse(get_proj_matrix(i)) * get_view_proj_matrix(i);
}

glm::mat4 GenericTechnique::get_proj_matrix(int i)
{
	return m_eval_config.projectionMatrix;
}

const std::vector<glm::mat4>& GenericTechnique::get_all_view_proj_matrices()
{
	return m_eval_config.sets_of_matrices[m_activeSet];
}

std::vector<glm::mat4> GenericTechnique::get_view_proj_matrices_range(int beginIdx, int endIdx)
{
	auto begin = m_eval_config.sets_of_matrices[m_activeSet].begin() + beginIdx;
	auto end = m_eval_config.sets_of_matrices[m_activeSet].begin() + endIdx;
	std::vector<glm::mat4> result(begin, end);
	return result;
}

const glm::mat4* GenericTechnique::get_view_proj_matrices_data_ptr()
{
	return m_eval_config.sets_of_matrices[m_activeSet].data();
}
 
TextureAndLayer GenericTechnique::get_nth_result_texture_info(int index, int range)
{
	switch (m_technique_config.m_type_of_medium)
	{
	case ViewTechnique::MultiViewMedium::SEPARATE_TEXTURES:
		return { textures_for_range(range)[index], 0, false };
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY:
		{
			const auto texIndex = index / m_views_per_target;
			const auto layerIndex = index - (texIndex * m_views_per_target);
			return { textures_for_range(range)[texIndex], static_cast<int>(layerIndex), true };
		}
	case ViewTechnique::MultiViewMedium::GIANT_TEXTURE:
		return { textures_for_range(range)[0], 0, false };
	case ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES:
		{
			auto numPerDrawcall = get_view_matrices_count();
			if (m_technique_config.m_max_num_gtex_tiles.has_value()) {
				numPerDrawcall = std::min(numPerDrawcall, m_technique_config.m_max_num_gtex_tiles.value());
			}
		
			//assert(m_technique_config.m_max_num_gtex_tiles.has_value());
			//const auto gtexIndex = index / m_technique_config.m_max_num_gtex_tiles.value();
			
			const auto gtexIndex = index / numPerDrawcall;
			return { textures_for_range(range)[gtexIndex], 0, false };
		}
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES:
		{
			auto cfg = resources()->getTextureConfig(textures_for_range(range)[0]);
			auto numLayers = static_cast<int>(cfg.num_layers);
			auto layerIndex = index % numLayers;
			return { textures_for_range(range)[0], layerIndex, true };
		}
	case ViewTechnique::MultiViewMedium::MULTIPLE_TEXTURE_ARRAYS:
		{
			const auto numPerDrawcall = max_num_views_per_draw_call();
			const auto targetIndex = index / numPerDrawcall;
			const auto layerIndex = static_cast<int>(index % numPerDrawcall);
			return { textures_for_range(range)[targetIndex], layerIndex, true };
		}
	default:
		assert(false); throw std::domain_error("wtf?!");
	}
}
TextureAndLayer GenericTechnique::get_nth_depth_texture_info(int index, int range)
{
	switch (m_technique_config.m_type_of_medium)
	{
	case ViewTechnique::MultiViewMedium::SEPARATE_TEXTURES:
		return { renderbuffers_for_range(range)[index], 0, false };
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY:
		{
			const auto texIndex = index / m_views_per_target;
			const auto layerIndex = index - (texIndex * m_views_per_target);
			return { renderbuffers_for_range(range)[texIndex], static_cast<int>(layerIndex), true };
		}
	case ViewTechnique::MultiViewMedium::GIANT_TEXTURE:
		return { renderbuffers_for_range(range)[0], 0, false };
	case ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES:
		{
			auto numPerDrawcall = get_view_matrices_count();
			if (m_technique_config.m_max_num_gtex_tiles.has_value()) {
				numPerDrawcall = std::min(numPerDrawcall, m_technique_config.m_max_num_gtex_tiles.value());
			}
		
			//assert(m_technique_config.m_max_num_gtex_tiles.has_value());
			//const auto gtexIndex = index / m_technique_config.m_max_num_gtex_tiles.value();
			
			const auto gtexIndex = index / numPerDrawcall;
			return { renderbuffers_for_range(range)[gtexIndex], 0, false };
		}
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES:
		{
			auto cfg = resources()->getTextureConfig(renderbuffers_for_range(range)[0]);
			auto numLayers = static_cast<int>(cfg.num_layers);
			auto layerIndex = index % numLayers;
			return { renderbuffers_for_range(range)[0], layerIndex, true };
		}
	case ViewTechnique::MultiViewMedium::MULTIPLE_TEXTURE_ARRAYS:
		{
			const auto numPerDrawcall = max_num_views_per_draw_call();
			const auto targetIndex = index / numPerDrawcall;
			const auto layerIndex = static_cast<int>(index % numPerDrawcall);
			return { renderbuffers_for_range(range)[targetIndex], layerIndex, true };
		}
	default:
		assert(false); throw std::domain_error("wtf?!");
	}
}


glm::ivec2 GenericTechnique::get_nth_result_offsets(int index)
{
	switch (m_technique_config.m_type_of_medium)
	{
	case ViewTechnique::MultiViewMedium::GIANT_TEXTURE:
	case ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES:
		return calculate_gianttex_viewportoffsets(index);
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES:
		return calculate_array_of_gianttex_offsets(index);
	default:
		return RenderPipeline::get_nth_result_offsets(index);
	}
}

glm::ivec2 GenericTechnique::get_nth_result_range(int index)
{
	switch (m_technique_config.m_type_of_medium)
	{
	case ViewTechnique::MultiViewMedium::GIANT_TEXTURE:
	case ViewTechnique::MultiViewMedium::MULTIPLE_GIANT_TEXTURES:
	case ViewTechnique::MultiViewMedium::TEXTURE_ARRAY_OF_GIANT_TEXTURES:
		return { width(), height() }; // TODO: What is this? What to do here for the multi-gtex configs?
	default:
		return RenderPipeline::get_nth_result_range(index);
	}
}

std::array<std::string, 3> GenericTechnique::getShaders()
{
	if (ViewTechnique::MultiViewPassThrough::ENABLED == m_technique_config.m_pass_through_settings) {
		if (trim_spaces(m_geom).length() < std::string("main(){}").length()) {
			throw std::logic_error("The technique has MultiViewPassThrough enabled, but there is no/an empty geometry shader!");
		}
		if (m_geom.find("#extension GL_NV_geometry_shader_passthrough : require") == std::string::npos) {
			throw std::logic_error("The technique has MultiViewPassThrough enabled, but it appears that the #extension is not declared as being required in the geometry shader!");
		}
	}
	else {
		if (m_geom.find("#extension GL_NV_geometry_shader_passthrough : require") != std::string::npos) {
			std::cout << "\nWARNING: The technique does not have MultiViewPassThrough enabled, but the #extension declared in the geometry shader!\n" << std::endl;
		}
	}

	if (ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW == m_technique_config.m_duplication_strategy) {
		if (m_vert.find("#extension GL_OVR_multiview : require") == std::string::npos && m_vert.find("#extension GL_OVR_multiview2 : require") == std::string::npos) {
			throw std::logic_error("The technique has MultiViewChange::NV_VIEW as the view change strategy set, but it appears that the #extension is not declared as being required in the vertex shader!");
		}
	}

	return std::array { m_vert, m_geom, m_frag };
}

RenderResources::ViewportConfig GenericTechnique::calculate_gianttex_viewportconfig(int matrixId)
{
	auto gtexIndex = 0;
	if (m_technique_config.m_max_num_gtex_tiles.has_value()) {
		gtexIndex = matrixId / m_technique_config.m_max_num_gtex_tiles.value();
	}
	auto cfg = resources()->getTextureConfig(textures_for_range(0)[gtexIndex]);
	
	matrixId -= gtexIndex * m_technique_config.m_max_num_gtex_tiles.value_or(1); // IF we have multiple giant textures
	
	auto perRow = cfg.width / width();
	auto fullRows = static_cast<uint32_t>(matrixId) / perRow;
	auto remainderInRow = static_cast<uint32_t>(matrixId) - (fullRows * perRow);
	return RenderResources::ViewportConfig{ remainderInRow * widthf(), fullRows * heightf(), widthf(), heightf() };
}

glm::ivec2 GenericTechnique::calculate_gianttex_viewportoffsets(int matrixId)
{
	auto numPerDrawcall = get_view_matrices_count();
	if (m_technique_config.m_max_num_gtex_tiles.has_value()) {
		numPerDrawcall = std::min(numPerDrawcall, m_technique_config.m_max_num_gtex_tiles.value());
	}
	
	const auto gtexIndex = matrixId / numPerDrawcall;
	auto cfg = resources()->getTextureConfig(textures_for_range(0)[gtexIndex]);

	matrixId -= gtexIndex * numPerDrawcall; // IF we have multiple giant textures
	
	auto perRow = cfg.width / width();
	auto fullRows = static_cast<uint32_t>(matrixId) / perRow;
	auto remainderInRow = static_cast<uint32_t>(matrixId) - (fullRows * perRow);
	return { remainderInRow * width(), fullRows * height() };
}

glm::ivec2 GenericTechnique::calculate_array_of_gianttex_offsets(int matrixId)
{
	// There is only one texture => get its config!
	auto cfg = resources()->getTextureConfig(textures_for_range(0)[0]);

	auto numLayers = static_cast<int>(cfg.num_layers);
	auto layerIndex = matrixId % numLayers;
	auto actualMaxPerLayer = get_view_matrices_count() / numLayers;
	if (get_view_matrices_count() % numLayers != 0) {
		actualMaxPerLayer += 1;
	}

	auto gtexIndex = matrixId / numLayers;
	auto perRow = cfg.width / width();
	auto fullRows = static_cast<uint32_t>(gtexIndex) / perRow;
	auto remainderInRow = static_cast<uint32_t>(gtexIndex) - (fullRows * perRow);
	return { remainderInRow * width(), fullRows * height() };
}

uint32_t GenericTechnique::max_concurrent_viewports() const
{
	static uint32_t max_viewports = []()
	{
		GLint param = 0;
		GL_CALL(glGetIntegerv(GL_MAX_VIEWPORTS, &param));
		return static_cast<uint32_t>(param);
	}();
	return max_viewports;
}

uint32_t GenericTechnique::max_num_views_per_draw_call() const
{
	auto limit = static_cast<uint32_t>(get_view_matrices_count());
	limit = std::min(limit, static_cast<uint32_t>(m_technique_config.m_max_num_views_to_produce_with_one_draw_call.value_or(std::numeric_limits<int>::max())));
	if (ViewTechnique::MultiViewDuplication::OVR_MULTIVIEW == m_technique_config.m_duplication_strategy) {
		limit = std::min(limit, 32u);
	}
	// TODO: How to really determine for all cases if viewports are to be drawn concurrently?
	if (ViewTechnique::MultiViewChange::VIEWPORT == m_technique_config.m_view_change_strategy && ViewTechnique::MultiViewDuplication::MULTIDRAW == m_technique_config.m_duplication_strategy) {
		limit = std::min(limit, max_concurrent_viewports());
	}
	return limit;
}

bool GenericTechnique::activate_next_set()
{
	if (m_activeSet < m_eval_config.sets_of_matrices.size() - 1) {
		m_activeSet++;
		return true;
	}
	return false;
}

bool GenericTechnique::activate_prev_set()
{
	if (m_activeSet > 0) {
		m_activeSet--;
		return true;
	}
	return false;
}

