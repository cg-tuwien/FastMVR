#include "SceneLoader.h"
#include "RenderApp.h"
#include "RenderResourcesGL.h"
#include "RenderMode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "OutputRerouter.h"
#include "GenericTechnique.h"
#include "string_utils.h"
#include "GeometryProviderEverything.h"
#include "GeometryProviderCubified.h"
#include "hash_utils.h"
#include "GeometryProviderCache.h"
#include "view_matrix_configs.h"
#include <random>
#include "predefined_matrix_configs.h"

int main(int argc, char* argv[])
{
	//try 
	//{
		// Class that pipes cout to the Visual Studio output window. 
		// Comment if not desired. 
		// Cout2VisualStudioDebugOutput c2v;

		// First parameter is the scene file to load (only one supported for now).
		if (argc < 2) {
			throw std::runtime_error("No filename provided");
		}
		// Scene loading
		std::cout << "Attempting to load scene from " << argv[1] << std::endl;

		uint32_t materials, objects, triangles;
		std::string sceneName = argv[1];
		SceneLoader loader(sceneName.c_str());
		loader.readScene(materials, objects, triangles);
		auto& vertices = loader.getSceneGeometry();
		auto& baked_vertices = loader.getSceneGeometryBaked();
		auto& indices = loader.getSceneIndices();
		auto& objs = loader.getObjects();
		std::cout << "Finished loading" << std::endl;

		// Create them now, but init them later
		GeometryProviderEverything geometryEverything;
		GeometryProviderCubified geometryCubifiedToBeCached100cubes {1000,	true};
		GeometryProviderCubified geometryCubifiedToBeCached1000cubes{50000,	true};
		GeometryProviderCache geometryCubified100cubesCached(geometryCubifiedToBeCached100cubes);
		GeometryProviderCache geometryCubified1000cubesCached(geometryCubifiedToBeCached1000cubes);
		auto geometryForRenderingToScreen = std::ref(geometryEverything);
		auto geometryForAncientCollectPipelines = std::ref(geometryEverything);
#ifdef SHADOW_MAPPING
		GeometryProviderEverything geometryQuad;
		geometryForRenderingToScreen = std::ref(geometryQuad);
#endif

	if (argc < 3) {
			throw std::runtime_error("No resolution provided");
		}
		// App parameters and setup
		std::string resolutionStr = argv[2];
		auto xPos = resolutionStr.find('x');
		if (xPos == std::string::npos) {
			throw std::runtime_error("Resolution not provided in the correct format (which would be WIDTHxHEIGHT)");
		}
		int width = std::stoi(resolutionStr.substr(0, xPos));
		int height = std::stoi(resolutionStr.substr(xPos + 1));
		static RenderApp app(width, height, geometryForRenderingToScreen);
		app.addAdditionalGeometryProviders(geometryCubified100cubesCached);
		app.addAdditionalGeometryProviders(geometryCubified1000cubesCached);
		
		// -- Done with parameter 2 a.k.a. argv[2]

		// Continue with parameter argv[1] => scene initialization after loading (Needed to create the OpenGL context before, however, which is done in RenderApp)
		std::cout << "Creating render resources" << std::endl;
		// Create and handle all the GL resources:
		RenderResourcesGL resources(objs);
		geometryEverything.initWithBakedData(baked_vertices, indices, sceneName);
		// We are going to use the geometry which contains everything in a big buffer for the following (nothing else is supported anyways, currently):
		resources.buildMarkedPrimitivesBuffer(geometryForRenderingToScreen);

#ifdef SHADOW_MAPPING
		std::vector<SceneVertex> quadvertices = {
			SceneVertex { {-1.0f, -1.0f, 0.5f}, glm::vec3{}, 0u },
			SceneVertex { { 1.0f, -1.0f, 0.5f}, glm::vec3{}, 0u },
			SceneVertex { { 1.0f,  1.0f, 0.5f}, glm::vec3{}, 0u },
			SceneVertex { { 1.0f,  1.0f, 0.5f}, glm::vec3{}, 0u },
			SceneVertex { {-1.0f,  1.0f, 0.5f}, glm::vec3{}, 0u },
			SceneVertex { {-1.0f, -1.0f, 0.5f}, glm::vec3{}, 0u }
		};
		std::vector<uint32_t> quadindices = {0, 1, 2, 3, 4, 5};
		geometryQuad.initWithBakedData(quadvertices, quadindices, "-");
#endif
	
		// -- Done with parameter 1 a.k.a. argv[1]

		if (argc < 4) {
			throw std::runtime_error("No view matrices config name provided");
		}
		// @TODO: Add checks to make far plane automatically adapt to scene for testing.

		// View position cameras
		float far_plane = 100; // Works for robot lab

		const std::string viewMatricesConfigName = argv[3];

		if (viewMatricesConfigName.find("robot_lab") != std::string::npos) {
			add_robot_lab_matrix_configs0();
			add_robot_lab_matrix_configs1();
			add_robot_lab_matrix_configs2();
			add_robot_lab_matrix_configs3();
			far_plane = 70;
		}
		else if (viewMatricesConfigName.find("viking_village") != std::string::npos) {
			add_viking_village_matrix_configs0();
			add_viking_village_matrix_configs1();
			add_viking_village_matrix_configs2();
			add_viking_village_matrix_configs3();
			far_plane = 450;
		}
		else if (viewMatricesConfigName.find("sponza") != std::string::npos) {
			add_sponza_matrix_configs0();
			add_sponza_matrix_configs1();
			add_sponza_matrix_configs2();
			add_sponza_matrix_configs3();
			far_plane = 60;
		}
		else if (viewMatricesConfigName.find("bistro") != std::string::npos) {
			add_bistro_matrix_configs0();
			add_bistro_matrix_configs1();
			add_bistro_matrix_configs2();
			add_bistro_matrix_configs3();
			far_plane = 600;
		}
		else if (viewMatricesConfigName.find("gallery") != std::string::npos) {
			add_gallery_matrix_configs0();
			add_gallery_matrix_configs1();
			add_gallery_matrix_configs2();
			add_gallery_matrix_configs3();
			far_plane = 60;
		}
		else if (viewMatricesConfigName.find("san_miguel") != std::string::npos) {
			add_san_miguel_matrix_configs0();
			add_san_miguel_matrix_configs1();
			add_san_miguel_matrix_configs2();
			add_san_miguel_matrix_configs3();
			far_plane = 60;
		}
		else {
			add_more_matrix_configs();
		}
	
		glm::mat4 pM = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, far_plane);
		
		auto view_positions = get_all_view_matrix_configs_with_name(viewMatricesConfigName);
		if (view_positions.empty()) {
			throw std::runtime_error("No matrices available under the given config name ('viewMatricesConfigName')");
		}

		// also apply projection- and model-matrix to all of these view-matrices
		std::vector<std::vector<glm::mat4>> matrices_per_view_position;
#if defined(SHADOW_MAPPING) && !defined(GBUFFER)
		assert(view_positions.size() >= 32 * 3);
		auto m = view_positions[0].size();
		int mIdx = static_cast<int>(m / 2) - static_cast<int>(static_cast<int>(sqrt(m)) / 2);
		if (m == 32 && viewMatricesConfigName.find("8x4") != std::string::npos) {
			mIdx = static_cast<int>(m / 2) - 4;
		}
		assert(mIdx >= 0 && mIdx < view_positions[0].size());
		for (int i = 0; (i + 32) < view_positions.size(); i += 32) {
			auto& cur_view_pos = matrices_per_view_position.emplace_back();
			for(int j = 0; j < m; ++j) {
				cur_view_pos.push_back(pM * view_positions[i + j][mIdx]);
			}
		}
#else
		for (size_t i = 0; i < view_positions.size(); ++i) {
			auto& view_position = view_positions[i];
			auto& cur_view_pos = matrices_per_view_position.emplace_back();
			for (auto& vM : view_position) {
				cur_view_pos.push_back(pM * vM);
			}
#ifdef FEWER
			i += 2;
#ifdef GBUFFER
			i += 2;
#endif
#endif
		}
#endif
		
		// -- Done with parameter 3 a.k.a. argv[3]

		if (argc < 5) {
			throw std::runtime_error("Not specified how to measure => 'gpu_time' vs 'cpu_time'");
		}
		const std::string measurementAndLoadConfig = argv[4];
		std::string measureTime = measurementAndLoadConfig;
		std::string loadConfig = "";
		{
			auto plusPos = measurementAndLoadConfig.find('+');
			if (std::string::npos != plusPos) {
				measureTime = measurementAndLoadConfig.substr(0, plusPos);
				loadConfig = measurementAndLoadConfig.substr(plusPos + 1);
			}
		}
		if ("cpu_time" != measureTime && "gpu_time" != measureTime) {
			throw std::runtime_error("Invalid measurementTime description specified! Must be either 'gpu_time' or 'cpu_time'.");
		}
		if ("" != loadConfig && "heavy_vertex_load" != loadConfig) {
			throw std::runtime_error("Invalid loadConfig description specified! Must either be not present or 'heavy_vertex_load'.");
		}
		measurement_type measurementType = "cpu_time" == measureTime ? measurement_type::cpu_time : measurement_type::gpu_time;
		load_config loadConfigType = "heavy_vertex_load" == loadConfig ? load_config::heavy_vertex_load : load_config::nothing;

		resources.makeHeavy = loadConfigType == load_config::heavy_vertex_load;
		
		// -- Done with all the command line parameters

		// Get the COMPUTERNAME and initialize the results output directory which is built up of the following sub-directories:
		//  ....something.../computerName/sceneName/viewMatricesConfigName/resolutionStr/measureTime/
		// Example:
		//   ../../../results/TEROKNOR/robot_lab_unity/arc_16matrices/800x600/cpu_time/
		char *temp_computername = nullptr;
	    std::string computerName = "UnknownPC";
	    temp_computername = getenv("COMPUTERNAME");
	    if (temp_computername != nullptr) {
	        computerName = temp_computername;
	        temp_computername = nullptr;
	    }

		if (argc < 6) {
			throw std::runtime_error("No results root directory specified.");
		}
		const std::string resultsRoot = argv[5];
		
		const std::string resultsOutputDirectory =	resultsRoot + "/" 
													+ computerName + "/" 
													+ extract_filename_without_extension(sceneName) + "/"
													+ viewMatricesConfigName + "/"
													+ resolutionStr + "/"
													+ measurementAndLoadConfig + "/";
		app.setResultsOutputDirectory(resultsOutputDirectory);

		// Experimental view and layering setup
		// @TODO: should best be moved to a config file that is passed as parameter.
		// Ideally, have multiple view base positions and sample each with a grid of views
		// distributed on the XY plane of a (randomly rotated) view cell.

		// TODO: Support depth ranges
		//// Depth ranges to sort into (for the techniques that support them).
		//// If we take the log2 of the view positions, we get the number of layers such that with each new layer,
		//// the amount of view positions can be halfed.
		//std::vector<glm::vec2> ranges;
		//int splits = (int)(log2(matrices.size()) + 1);
		//for (int i = 0; i < splits; i++)
		//{	ranges.push_back({ (i * far_plane) / splits, ((i + 1) * far_plane) / splits });	}

		// Evaluation config. Feel free to modify and extend with properties that make sense.
		EvalConfig eval_conf;
		eval_conf.resolution_x = width;
		eval_conf.resolution_y = height;
		//eval_conf.depth_ranges = ranges;
		eval_conf.sets_of_matrices = matrices_per_view_position;
		eval_conf.orig_sets_of_matrices = matrices_per_view_position;
		eval_conf.projectionMatrix = pM;
		eval_conf.verify_equal_number_of_matrices_per_view_position();

		using MVD = ViewTechnique::MultiViewDuplication;
		using MVM = ViewTechnique::MultiViewMedium;
		using MVC = ViewTechnique::MultiViewChange;
		using MVP = ViewTechnique::MultiViewPassThrough;
		using DUP = duplication_strategy;

		using DLS = DepthTechnique::DepthLayerSorting;
		using DLM = DepthTechnique::DepthLayerMedium;
		using DLC = DepthTechnique::DepthLayerChange;

		std::cout << "Running" << std::endl;

		app.setResources(&resources);

		//////////////////////////////////////////////////////////////
		// EXPERIENCE THE POWER OF THE ALL-MIGHTY GENERIC TECHNIQUE
		// 
		// Possible TechniqueConfig parameters and their right order:
		// {MVD::, MVM::, MVC::, MVP::}

		// =========== BEGIN SECTION 1 ============
		// Can only render the entire scene at once.
		// Can only render the entire depth range at once.
		// Assumes that model matrices are baked into the geometry.
		// vvvv
		TechniqueConfig techConf;
		const int x4num = 4;
		assert(4 == x4num);
	
		// Uses separate pass and framebuffer with separate textures attached for each view point rendering.
		// Uses a simple uniform to set the view matrix in each pass.
		techConf = TechniqueConfig{MVD::MULTIPASS, MVM::SEPARATE_TEXTURES, MVC::FRAMEBUFFER}.setMaximumNumberOfViewsToProduceWithOneDrawCall(1);
		std::array multipass_separate_textures_aka_rp001 = {
			GenericTechnique("m-pass", techConf, eval_conf, geometryEverything,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-pass", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-pass", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")),
		};

		// Uses multi-pass to render all view points, renders to a GIANT TEXTURE.
		// Viewport is set before rendering each view.
		// Updates view matrix with each pass.
		techConf = TechniqueConfig{MVD::MULTIPASS, MVM::GIANT_TEXTURE, MVC::VIEWPORT}.setMaximumNumberOfViewsToProduceWithOneDrawCall(1);
		std::array multipass_into_gianttex_aka_rp402 = {
			GenericTechnique("m-pass_g-tex", techConf, eval_conf, geometryEverything,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
			
			GenericTechnique("m-pass_g-tex", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
			
			GenericTechnique("m-pass_g-tex", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
		};

		techConf = TechniqueConfig{MVD::MULTIPASS, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_VIEWPORT}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())))
					.setMaximumNumberOfTilesInGiantTexture(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array multipass_into_multiple_small_gianttextures_x4 = {
			GenericTechnique(fmt::format("m-pass_g-tex_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
			
			GenericTechnique(fmt::format("m-pass_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
			
			GenericTechnique(fmt::format("m-pass_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v.vert"), "", from_file("primitiveid_out.frag")), // same shaders as unoptimized_aka_rp001
		};

		// Uses one pass for all view points, renders to a GIANT TEXTURE.
		// Duplicates by using multi-draw call.
		// Geometry shader selects viewport for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = {MVD::MULTIDRAW, MVM::GIANT_TEXTURE, MVC::VIEWPORT, MVP::DISABLED};
		std::array multidraw_into_gianttex_singlepass = {
			GenericTechnique ("m-draw_g-tex", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique ("m-draw_g-tex", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique ("m-draw_g-tex", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"))
		};
		std::array multidraw_into_gianttex_singlepass_with_gcull = {
			GenericTechnique ("m-draw+gcull_g-tex", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("m-draw+gcull_g-tex", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("m-draw+gcull_g-tex", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING"))
		};

		// Same as above, but renders into GIANT TEXTURES consisting of 4 tiles, each
		techConf = TechniqueConfig{MVD::MULTIDRAW, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_VIEWPORT, MVP::DISABLED}
					.setMaximumNumberOfTilesInGiantTexture(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())))
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array multidraw_into_gianttex_singlepass_x4 = {
			GenericTechnique (fmt::format("m-draw_g-tex_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique (fmt::format("m-draw_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique (fmt::format("m-draw_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"))
		};
		std::array multidraw_into_gianttex_singlepass_with_gcull_x4 = {
			GenericTechnique (fmt::format("m-draw+gcull_g-tex_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique (fmt::format("m-draw+gcull_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique (fmt::format("m-draw+gcull_g-tex_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING"))
		};

		// Uses one pass for all view points, renders to a GIANT TEXTURE.
		// Duplicates by using multi-draw call.
		// Geometry shader selects viewport for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		// Speeds things up by using PASS-THROUGH (NVIDIA only).
		techConf = TechniqueConfig{MVD::MULTIDRAW, MVM::GIANT_TEXTURE, MVC::VIEWPORT}.setGeometryShaderPassThroughOptimizationToEnabled();
		std::array multidraw_into_gianttex_singlepass_passthrough_aka_rp401 = {
			GenericTechnique("m-draw++_g-tex", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-draw++_g-tex", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-draw++_g-tex", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
		};

		// 1st implementation of two approaches presented in "GPU-based multi-view rendering" by Sorbier et al.
		//   Duplicates in a geometry shader loop
		//   Uses a uniform buffer to set all view matrices at once
		//   Stores results in a giant texture
		//   Performs clipping in fragment shader only
		techConf = {MVD::GEOMETRY_SHADER_LOOP, MVM::GIANT_TEXTURE, MVC::NONE};
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_cull_in_frag = {
			GenericTechnique("sorbier_frag", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("sorbier_frag", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("sorbier_frag", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
		};

		// 2nd implementation of two approaches presented in "GPU-based multi-view rendering" by Sorbier et al.
		//   Duplicates in a geometry shader loop
		//   Uses a uniform buffer to set all view matrices at once
		//   Stores results in a giant texture
		//   Performs clipping in both, geometry shader and fragment shader
		techConf = {MVD::GEOMETRY_SHADER_LOOP, MVM::GIANT_TEXTURE, MVC::NONE};
		techConf.m_requires_fragment_shader_for_depth_only = true;	
		std::array sorbier_cull_in_both = {
			GenericTechnique ("sorbier_geom+frag", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING")),
		};

		// Sorbier-variant which uses (potentially multiple) not-so-giant-textures of 2x1 tiles and potentially multiple passes.
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_NONE}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(2)
						.setMaximumNumberOfTilesInGiantTexture(2);
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_cull_in_both_x2_mpass = {
			GenericTechnique ("sorbier_geom+frag_x2", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 2"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x2", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 2"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x2", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 2"))
			          .to_geom("#define GEOM_CULLING")),
		};

		// Sorbier-variant which uses (potentially multiple) not-so-giant-textures of 2x2 tiles and potentially multiple passes.
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_NONE}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())))
						.setMaximumNumberOfTilesInGiantTexture(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_cull_in_both_x4_mpass = {
			GenericTechnique (fmt::format("sorbier_geom+frag_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique (fmt::format("sorbier_geom+frag_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique (fmt::format("sorbier_geom+frag_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING")),
		};
		std::array sorbier_cull_in_both_with_bfcull_x4_mpass = {
			GenericTechnique (fmt::format("sorbier_geom+frag+bfc_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique (fmt::format("sorbier_geom+frag+bfc_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique (fmt::format("sorbier_geom+frag+bfc_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
			          .to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
		};

		// Sorbier-variant which uses (potentially multiple) not-so-giant-textures of 3x2 tiles and potentially multiple passes.
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_NONE}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(6)
						.setMaximumNumberOfTilesInGiantTexture(6);
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_cull_in_both_x6_mpass = {
			GenericTechnique ("sorbier_geom+frag_x6", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 6"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x6", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 6"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x6", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 6"))
			          .to_geom("#define GEOM_CULLING")),
		};

		// Sorbier-variant which uses (potentially multiple) not-so-giant-textures of 3x3 tiles and potentially multiple passes.
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::MULTIPLE_GIANT_TEXTURES, MVC::FRAMEBUFFER_THEN_NONE}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(8)
						.setMaximumNumberOfTilesInGiantTexture(8);
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_cull_in_both_x8_mpass = {
			GenericTechnique ("sorbier_geom+frag_x8", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 8"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x8", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 8"))
			          .to_geom("#define GEOM_CULLING")),
			
			GenericTechnique ("sorbier_geom+frag_x8", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS 8"))
			          .to_geom("#define GEOM_CULLING")),
		};

		// Adaptation of the sorbier-techniques to use clip planes, i.e. gl_ClipDistance
		techConf = {MVD::GEOMETRY_SHADER_LOOP, MVM::GIANT_TEXTURE, MVC::NONE};
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array sorbier_clip_planes = {
			GenericTechnique("sorb_planes", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_all("#define PLANE_CLIPPING")),
			
			GenericTechnique("sorb_planes", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_all("#define PLANE_CLIPPING")),
			
			GenericTechnique("sorb_planes", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_all("#define PLANE_CLIPPING")),
		};
		std::array sorbier_cull_in_geom_clip_planes = {
			GenericTechnique ("sorb_geom+planes", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING").to_all("#define PLANE_CLIPPING")),
			
			GenericTechnique ("sorb_geom+planes", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING").to_all("#define PLANE_CLIPPING")),
			
			GenericTechnique ("sorb_geom+planes", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_gianttex.geom"), from_file("primitiveid_out_subarea.frag"), 
			add_code().to_all(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))
			          .to_geom("#define GEOM_CULLING").to_all("#define PLANE_CLIPPING")),
		};

		// Uses an array texture and geometry shader to select layer for each view point rendering.
		// Geometry shader duplicates geometry in loop.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = {MVD::GEOMETRY_SHADER_LOOP, MVM::TEXTURE_ARRAY, MVC::LAYER};
		std::array geometry_shader_multiplication_aka_rp202 = {
			GenericTechnique("geom-mult", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("geom-mult", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("geom-mult", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
		};
		std::array geometry_shader_multiplication_with_gcull = {
			GenericTechnique("gmult+gcull", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("gmult+gcull", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("gmult+gcull", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_geom("#define GEOM_CULLING")),
		};

		// same same as above, but only produce 4 layers in one draw call
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::TEXTURE_ARRAY, MVC::LAYER}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array geometry_shader_multiplication_x4s = {
			GenericTechnique(fmt::format("geom-mult_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("geom-mult_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("geom-mult_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
		};
		std::array geometry_shader_multiplication_with_gcull_x4s = {
			GenericTechnique(fmt::format("gmult+gcull_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
		};

		// same same as above, but use multiple layered textures
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_LOOP, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array geometry_shader_multiplication_x4 = {
			GenericTechnique(fmt::format("geom-mult_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("geom-mult_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("geom-mult_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
		};
		std::array geometry_shader_multiplication_with_gcull_x4 = {
			GenericTechnique(fmt::format("gmult+gcull_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING")),
		};
		std::array geometry_shader_multiplication_with_gcull_and_bfc_x4 = {
			GenericTechnique(fmt::format("gmult+gcull+bfc_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull+bfc_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique(fmt::format("gmult+gcull+bfc_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("multiply_geometry_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value())).to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
		};

		// Uses one pass for each view point, renders to an array texture.
		// GEOMETRY SHADER with PASS THROUGH selects layer for each view point rendering.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = TechniqueConfig{MVD::MULTIPASS, MVM::TEXTURE_ARRAY, MVC::LAYER}.setMaximumNumberOfViewsToProduceWithOneDrawCall(1).setGeometryShaderPassThroughOptimizationToEnabled();
		std::array multipass_into_layers_aka_rp203 = {
			GenericTechnique("m-pass++_layers", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_layer.vert"), from_file("select_layer_via_uniform_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-pass++_layers", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_layer.vert"), from_file("select_layer_via_uniform_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-pass++_layers", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_layer.vert"), from_file("select_layer_via_uniform_passthrough.geom"), from_file("primitiveid_out.frag")),
		};

		// Uses one pass for all view points, renders to an array texture.
		// Duplicates by creating a giant index buffer that consecutively references each triangle as often as there are views.
		// Geometry shader selects layer for each view point based on primitive ID.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = TechniqueConfig{MVD::GIANT_INDEX_BUFFER, MVM::TEXTURE_ARRAY, MVC::LAYER}.setIndexBufferDuplicationStrategy(duplication_strategy::interleaved);
		std::array giant_idxbfr_interleaved_aka_rp302 = { // This one has the worst performance of them all. ~ 7 seconds :O
			GenericTechnique("g-idx_interleaved", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("interleaved_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("g-idx_interleaved", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("interleaved_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("g-idx_interleaved", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("interleaved_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position()))),
		};

		// TODO-BUG: Only the first two layers get filled. I have no idea why.
		techConf = TechniqueConfig{MVD::GIANT_INDEX_BUFFER, MVM::TEXTURE_ARRAY, MVC::LAYER}.setIndexBufferDuplicationStrategy(duplication_strategy::consecutive);
		std::array giant_idxbfr_consecutive = {
			GenericTechnique("g-idx_consecutive", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("consecutive_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("g-idx_consecutive", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("consecutive_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("g-idx_consecutive", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("consecutive_idxbfr_into_layers.geom"), from_file("primitiveid_out.frag")),
		};

		// Uses one pass for all view points, renders to an array texture.
		// Duplicates by creating a via geometry shader instances.
		// Geometry shader selects layer for each view point based on invocation.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = {MVD::GEOMETRY_SHADER_INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER};
		std::array geom_instancing_aka_rp311 = {
			GenericTechnique("geom-inst_layer", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("geom-inst_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))),
			
			GenericTechnique("geom-inst_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))),
		};
		std::array geom_instancing_with_gcull = {
			GenericTechnique("ginst+gcull_layer", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("ginst+gcull_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("ginst+gcull_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING")),
		};
		std::array geom_instancing_with_plane_clipping = {
			GenericTechnique("ginst+pclip_layer", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique("ginst+pclip_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique("ginst+pclip_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define PLANE_CLIPPING")),
		};
		std::array geom_instancing_with_gcull_and_plane_clipping = {
			GenericTechnique("ginst+gcull+pclip_layer", techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique("ginst+gcull+pclip_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique("ginst+gcull+pclip_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", eval_conf.number_of_matrices_per_view_position()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
		};

		// Same as above, but in one pass, only produces a given number of targets (the postfix-'s' means "same texture")
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array geom_instancing_x4s = {
			GenericTechnique(fmt::format("ginst_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("ginst_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("ginst_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
		};
		std::array geom_instancing_with_gcull_x4s = {
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
		};
		std::array geom_instancing_with_plane_clipping_x4s = {
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
		};
		std::array geom_instancing_with_gcull_and_plane_clipping_x4s = {
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
		};
		
		// Same as above, but does not render into different layers but uses separate target textures à 4 layers
		techConf = TechniqueConfig{MVD::GEOMETRY_SHADER_INSTANCING, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array geom_instancing_x4 = {
			GenericTechnique(fmt::format("ginst_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("ginst_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
			
			GenericTechnique(fmt::format("ginst_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))),
		};
		std::array geom_instancing_with_gcull_x4 = {
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING")),
		};
		std::array geom_instancing_with_gcull_and_bfc_x4 = {
			GenericTechnique(fmt::format("ginst+gcull+bfc_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull+bfc_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique(fmt::format("ginst+gcull+bfc_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
		};
		std::array geom_instancing_with_plane_clipping_x4 = {
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+pclip_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define PLANE_CLIPPING")),
		};
		std::array geom_instancing_with_gcull_and_plane_clipping_x4 = {
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
			
			GenericTechnique(fmt::format("ginst+gcull+pclip_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("passthrough.vert"), from_file("geom_instancing.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom(fmt::format("#define NUM_INVOCATIONS {}", techConf.m_max_num_views_to_produce_with_one_draw_call.value()))
				.to_geom("#define GEOM_CULLING").to_geom("#define PLANE_CLIPPING")),
		};

	// Duplicates via instanced rendering.
		// Geometry shader selects layer for each view point based on instance ID.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = {MVD::INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER};
		std::array instancing_into_layers_aka_rp301 = {
			GenericTechnique("inst_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("inst_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("inst_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag")),
		};
	
		techConf = {MVD::INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER};
		std::array instancing_into_layers_with_gcull = {
			GenericTechnique("inst+gcull_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("inst+gcull_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("inst+gcull_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
		};
	
		techConf = TechniqueConfig{MVD::INSTANCING, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));;
		std::array instancing_into_layers_with_gcull_x4 = {
			GenericTechnique(fmt::format("inst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("inst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("inst+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
		};
	
		// Wilson's technique from that VR meetup, which was also mentioned by Alex Vlachos
		techConf = {MVD::INSTANCING, MVM::GIANT_TEXTURE, MVC::NONE};
		techConf.m_requires_fragment_shader_for_depth_only = true;
		std::array instancing_planes_into_gtex = {
			GenericTechnique("inst+planes_g-tex", techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), "", from_file("primitiveid_out_subarea.frag"),
			add_code()
				.to_vert(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_vert("#define PLANE_CLIPPING")
				.to_frag("#define PLANE_CLIPPING")),
			
			GenericTechnique("inst+planes_g-tex", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), "", from_file("primitiveid_out_subarea.frag"),
			add_code()
				.to_vert(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_vert("#define PLANE_CLIPPING")
				.to_frag("#define PLANE_CLIPPING")),
			
			GenericTechnique("inst+planes_g-tex", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), "", from_file("primitiveid_out_subarea.frag"),
			add_code()
				.to_vert(fmt::format("#define NUM_VIEWS {}", eval_conf.number_of_matrices_per_view_position())).to_vert("#define PLANE_CLIPPING")
				.to_frag("#define PLANE_CLIPPING")),
		};

		// Duplicates via instanced rendering.
		// Geometry shader selects layer for each view point based on instance ID.
		// Uses a uniform buffer to set all view matrices at once.
		// Speeds things up by using Pass-Through (NVIDIA only).
		techConf = TechniqueConfig{MVD::INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER}.setGeometryShaderPassThroughOptimizationToEnabled();
		std::array instancing_into_layers_passthrough_aka_rp301a = {
			GenericTechnique("inst++_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("inst++_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("inst++_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
		};

		// Duplicates via instanced rendering.
		// Geometry shader selects layer for each view point based on instance ID.
		// Uses a uniform buffer to set all view matrices at once.
		// Speeds things up by using Pass-Through (NVIDIA only).
		techConf = TechniqueConfig{MVD::INSTANCING, MVM::TEXTURE_ARRAY, MVC::LAYER}.setGeometryShaderPassThroughOptimizationToEnabled()
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array instancing_into_layers_passthrough_x4 = {
			GenericTechnique(fmt::format("inst++_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique(fmt::format("inst++_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique(fmt::format("inst++_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_instanced.vert"), from_file("select_layer_based_on_instance_passthrough.geom"), from_file("primitiveid_out.frag")),
		};

		// Duplicates by using multi-draw call.
		// Geometry shader selects layer for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = {MVD::MULTIDRAW, MVM::TEXTURE_ARRAY, MVC::LAYER, MVP::DISABLED};
		std::array multidraw_into_layers_aka_rp312 = {
			GenericTechnique("m-draw_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-draw_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),

			GenericTechnique("m-draw_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"))
		};
		std::array  multidraw_into_layers_with_gcull = {
			GenericTechnique("m-draw+gcull_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("m-draw+gcull_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),

			GenericTechnique("m-draw+gcull_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING"))
		};

		// Same as above, but _x4s
		techConf = TechniqueConfig{MVD::MULTIDRAW, MVM::TEXTURE_ARRAY, MVC::LAYER, MVP::DISABLED}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array multidraw_into_layers_x4s = {
			GenericTechnique(fmt::format("m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique(fmt::format("m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),

			GenericTechnique(fmt::format("m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"))
		};
		std::array  multidraw_into_layers_with_gcull_x4s = {
			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),

			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING"))
		};

		// Same as above, but _x4
		techConf = TechniqueConfig{MVD::MULTIDRAW, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER, MVP::DISABLED}
					.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array multidraw_into_layers_x4 = {
			GenericTechnique(fmt::format("m-draw_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique(fmt::format("m-draw_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),

			GenericTechnique(fmt::format("m-draw_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"))
		};
		std::array  multidraw_into_layers_with_gcull_x4 = {
			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING")),

			GenericTechnique(fmt::format("m-draw+gcull_layer_x{}", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"), 
			add_code().to_geom("#define GEOM_CULLING"))
		};

		// Duplicates by using multi-draw call.
		// Geometry shader selects layer for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		// Speeds things up by using Pass-Through (NVIDIA only).
		techConf = TechniqueConfig{MVD::MULTIDRAW, MVM::TEXTURE_ARRAY, MVC::LAYER}.setGeometryShaderPassThroughOptimizationToEnabled();
		std::array multidraw_into_layers_passthrough_aka_rp312a = {
			GenericTechnique("m-draw++_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-draw++_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("m-draw++_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
		};

		// Duplicates by using multi-draw call INDIRECT.
		// Geometry shader selects layer for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		techConf = TechniqueConfig{MVD::MULTIDRAW_INDIRECT, MVM::TEXTURE_ARRAY, MVC::LAYER};
		techConf.m_include_begin_in_measurement = true;
		std::array multidraw_indirect_into_layers = {
			GenericTechnique("ind_m-draw_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("ind_m-draw_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
			
			GenericTechnique("ind_m-draw_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
		};
		std::array multidraw_indirect_into_layers_with_gcull = {
			GenericTechnique("ind_m-draw+gcull_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("ind_m-draw+gcull_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING")),
			
			GenericTechnique("ind_m-draw+gcull_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING")),
		};
		std::array multidraw_indirect_into_layers_with_gcull_and_bfc = {
			GenericTechnique("ind_m-draw+gcull+bfc_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique("ind_m-draw+gcull+bfc_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
			
			GenericTechnique("ind_m-draw+gcull+bfc_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING").to_geom("#define BACKFACE_CULLING")),
		};

		// Same as above, but _x4s
		techConf = TechniqueConfig{MVD::MULTIDRAW_INDIRECT, MVM::TEXTURE_ARRAY, MVC::LAYER}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(x4num, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array multidraw_indirect_into_layers_x4s = {
			GenericTechnique(fmt::format("ind_m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique(fmt::format("ind_m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
			
			GenericTechnique(fmt::format("ind_m-draw_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
		};
		std::array multidraw_indirect_into_layers_with_gcull_x4s = {
			GenericTechnique(fmt::format("ind_m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ind_m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING")),
			
			GenericTechnique(fmt::format("ind_m-draw+gcull_layer_x{}s", x4num), techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS").to_geom("#define GEOM_CULLING")),
		};
	
		// Duplicates by using multi-draw call INDIRECT.
		// Geometry shader selects layer for each view point based on draw call.
		// Uses a uniform buffer to set all view matrices at once.
		// Speeds things up by using Pass-Through (NVIDIA only).
		techConf = TechniqueConfig{MVD::MULTIDRAW_INDIRECT, MVM::TEXTURE_ARRAY, MVC::LAYER}.setGeometryShaderPassThroughOptimizationToEnabled();
		techConf.m_include_begin_in_measurement = true;
		std::array multidraw_indirect_into_layers_passthrough_aka_rp313 = {
			GenericTechnique("ind_m-draw++_layer", techConf, eval_conf, geometryEverything,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag")),
			
			GenericTechnique("ind_m-draw++_layer", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
			
			GenericTechnique("ind_m-draw++_layer", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_of_drawid.vert"), from_file("select_layer_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag"),
			add_code().to_all("#define PASS_ON_GEOMETRY_ID").to_frag("#define MULTIPLE_MAPPING_BUFFERS")),
		};

		// Uses NVIDIA MVR to render all view points (4) at once, renders to array texture.
		// Uses uniform buffer to update view matrices all at once.
		techConf = {MVD::OVR_MULTIVIEW, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER};
		std::array ovr_multiview_aka_rp501 = {
			GenericTechnique("ovr_x32", techConf, eval_conf, geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 32ull)))),
			
			GenericTechnique("ovr_x32", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 32ull)))),
			
			GenericTechnique("ovr_x32", techConf, eval_conf,  
			geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 32ull)))),
		};

		techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(4, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array ovr_multiview_4xN = {
			GenericTechnique("ovr_x4", techConf, eval_conf, geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))),
			
			GenericTechnique("ovr_x4", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))),
			
			GenericTechnique("ovr_x4", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))),
		};

		techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::MULTIPLE_TEXTURE_ARRAYS, MVC::FRAMEBUFFER_THEN_LAYER}
						.setMaximumNumberOfViewsToProduceWithOneDrawCall(std::min(2, static_cast<int>(eval_conf.number_of_matrices_per_view_position())));
		std::array ovr_multiview_2xN = {
			GenericTechnique("ovr_x2", techConf, eval_conf, geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))),
			
			GenericTechnique("ovr_x2", techConf, eval_conf, geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))),
			
			GenericTechnique("ovr_x2", techConf, eval_conf, geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code().to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))),
		};

		{
			// The goal is to have one single draw call, make use of 2 hardware accelerated views, and render k views into one giant texture at the same time
			auto m = eval_conf.number_of_matrices_per_view_position();
			auto k = m / 2;
			if (m % 2 != 0) { k += 1; }
			techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::TEXTURE_ARRAY_OF_GIANT_TEXTURES, MVC::LAYER_THEN_NONE}
							.setMaximumNumberOfViewsToProduceWithOneDrawCall(2 * k)
							.setMaximumNumberOfTilesInGiantTexture(k);
		}
		std::array ovr_multiview_into_gianttex_with_planes_2xN = {
			GenericTechnique("ovr_x2_gtex_planes", techConf, eval_conf,  
			geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x2_gtex_planes", techConf, eval_conf,  
			geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x2_gtex_planes", techConf, eval_conf,  
			geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 2ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
		};
	
		{
			// The goal is to have one single draw call, make use of 4 hardware accelerated views, and render k views into one giant texture at the same time
			auto m = eval_conf.number_of_matrices_per_view_position();
			auto k = m / 4;
			if (m % 4 != 0) { k += 1; }
			techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::TEXTURE_ARRAY_OF_GIANT_TEXTURES, MVC::LAYER_THEN_NONE}
							.setMaximumNumberOfViewsToProduceWithOneDrawCall(4 * k)
							.setMaximumNumberOfTilesInGiantTexture(k);
		}
		std::array ovr_multiview_into_gianttex_with_planes_4xN = {
			GenericTechnique("ovr_x4_gtex_planes", techConf, eval_conf,  
			geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x4_gtex_planes", techConf, eval_conf,  
			geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x4_gtex_planes", techConf, eval_conf,  
			geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
		};

		{
			// The goal is to have one single draw call, make use of 6 hardware accelerated views, and render k views into one giant texture at the same time
			auto m = eval_conf.number_of_matrices_per_view_position();
			auto k = m / 6;
			if (m % 6 != 0) { k += 1; }
			techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::TEXTURE_ARRAY_OF_GIANT_TEXTURES, MVC::LAYER_THEN_NONE}
							.setMaximumNumberOfViewsToProduceWithOneDrawCall(6 * k)
							.setMaximumNumberOfTilesInGiantTexture(k);
		}
		std::array ovr_multiview_into_gianttex_with_planes_6xN = {
			GenericTechnique("ovr_x6_gtex_planes", techConf, eval_conf,  
			geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 6ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x6_gtex_planes", techConf, eval_conf,  
			geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 6ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
			
			GenericTechnique("ovr_x6_gtex_planes", techConf, eval_conf,  
			geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), "", from_file("primitiveid_out.frag"), add_code()
				.to_vert("#define PLANE_CLIPPING")
				.to_vert(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 6ull)))
				.to_vert(fmt::format("#define NUM_TILES {}", techConf.m_max_num_gtex_tiles.value()))
			),
		};

		{
			// The goal is to have one single draw call, make use of 4 hardware accelerated views, and render k views into one giant texture at the same time
			auto m = eval_conf.number_of_matrices_per_view_position();
			auto k = m / 4;
			if (m % 4 != 0) { k += 1; }
			techConf = TechniqueConfig{MVD::OVR_MULTIVIEW, MVM::TEXTURE_ARRAY_OF_GIANT_TEXTURES, MVC::LAYER_THEN_VIEWPORT}
							.setMaximumNumberOfViewsToProduceWithOneDrawCall(4 * k)
							.setMaximumNumberOfTilesInGiantTexture(k);
		}
		std::array ovr_multiview_into_gianttex_via_viewport_4xN = {
			GenericTechnique("ovr_x4_gtex_viewport", techConf, eval_conf,  
			geometryEverything,
			from_file("mult_v_viewid_ovr.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag"), add_code()
				.to_all(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_all("#define OVR_VIEWPORT 1")
			),
			
			GenericTechnique("ovr_x4_gtex_viewport", techConf, eval_conf,  
			geometryCubified100cubesCached,
			from_file("mult_v_viewid_ovr.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag"), add_code()
				.to_all(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_all("#define OVR_VIEWPORT 1")
			),
			
			GenericTechnique("ovr_x4_gtex_viewport", techConf, eval_conf,  
			geometryCubified1000cubesCached,
			from_file("mult_v_viewid_ovr.vert"), from_file("select_viewport_based_on_drawid_passthrough.geom"), from_file("primitiveid_out.frag"), add_code()
				.to_all(fmt::format("#define NUM_VIEWS {}", std::min(eval_conf.number_of_matrices_per_view_position(), 4ull)))
				.to_all("#define OVR_VIEWPORT 1")
			),
		};

		// ^^^^
		// Can only render the entire scene at once.
		// Can only render the entire depth range at once.
		// Assumes that model matrices are baked into the geometry.
		// ============ END OF SECTION 1 ============
		
		//glEnable(GL_REPRESENTATIVE_FRAGMENT_TEST_NV);

		bool autoModeEnabled = false;
		if (argc >= 7) {
			std::string autoStr = argv[6];
			if ("auto" == autoStr) {
				autoModeEnabled = true;
			}
		}

		int autoModeNumMeasurements = 3;
		if (argc >= 8) {
			std::string numMeasurements = argv[7];
			autoModeNumMeasurements = std::stoi(numMeasurements);
		}

		std::vector<RenderPipeline*> availablePipelines = 
		{
			//// --------- multi-pass techniques BEGIN ---------
			////
			//// Looks like all of these perform approximately the same
			//// There is no clear winner or so, they sometimes seem to
			//// swap positions rather arbitrarily.
			//// 
			//// IF there is a slight tendency, then giant textures seem to be slightly better.
			//// _x4 doesn't seem to matter in this case, however.
			
			//&multipass_separate_textures_aka_rp001[0],
			//&multipass_separate_textures_aka_rp001[1],
			//&multipass_separate_textures_aka_rp001[2],

			//&multipass_into_gianttex_aka_rp402[0],
			//&multipass_into_gianttex_aka_rp402[1],
			//&multipass_into_gianttex_aka_rp402[2],

			//&multipass_into_multiple_small_gianttextures_x4[0],
			//&multipass_into_multiple_small_gianttextures_x4[1],
			//&multipass_into_multiple_small_gianttextures_x4[2],

			//&multipass_into_layers_aka_rp203[0], // multipass + passthrough
			//&multipass_into_layers_aka_rp203[1],
			//&multipass_into_layers_aka_rp203[2],

			//// --------- multi-pass techniques END ---------

			
			//// --------- ATTENTION: The following techniques can not emit more than ~146 vertices in geometry shader (i.e. 6x6x3 = max) --------- 

			//// --------- Sorbier-based techniques BEGIN ---------
			////
			//// Original Sorbier is shit (as it appeared at first... trust in yourself! ^^).
			//// The only two positive things that can be said about it are:
			////  - It has the benefit of the geometry shader-savings with high vertex shader loads (as do all the other .geom-multiplication techniques)
			////  - It used to be the only way to render multiple views with one draw call back then when the paper was released 
			////
			//// The only reason why Sorbier remains, is the _x4-optimization.
			//// The improved memory-...handling?/layout?/streaming? is what makes it fast, regardless of the algorithm.
			//// To its credit: The idea of culling full triangles in the geometry shader comes from the Sorbier paper.

			//&sorbier_cull_in_both[0],
			//&sorbier_cull_in_both[1],
			//&sorbier_cull_in_both[2],

			&sorbier_cull_in_both_x4_mpass[0],
			&sorbier_cull_in_both_x4_mpass[1],
			&sorbier_cull_in_both_x4_mpass[2],

			//// --------- Sorbier-based techniques END ---------

			
			//// --------- Sorbier-based techniques END ---------

			//// --------- Geometry Shader Multiplication techniques BEGIN ---------
			////
			//// This is regarded to be a super-bad technique because it does so much work in the geometry shader... in a loop.
			//// However, it becomes pretty fast with high vertex shader loads. Therefore, it has to stay and be tested&evaluated.
			//// Furthermore, it is one of the super-compatible techniques which also runs on older AMD cards without any extensions.
			////
			//// The optimizations of triangle culling in geometry shader + _x4 speed it up tremendously.
			////
			//// It will be interesting to see, how it compares to geometry shader instancing. Can geom-instancing distribute the
			//// load on the GPU better? Or does the loop in the geometry shader not matter?
			//// Huh, that's interesting... first tests indicate that the loop might not even matter at all, compared to geom-instancing.
			//// In viking village, geometry shader multiplication is faster than geometry shader instancing. We'll see about the other scenes.
			
			//&geometry_shader_multiplication_aka_rp202[0],
			//&geometry_shader_multiplication_aka_rp202[1],
			//&geometry_shader_multiplication_aka_rp202[2],
			//// --------- NEW from 29.01.2020 onwards BEGIN ---------
			&geometry_shader_multiplication_with_gcull[0], // => + additional culling in the geometry shader
			&geometry_shader_multiplication_with_gcull[1],
			&geometry_shader_multiplication_with_gcull[2],
			&geometry_shader_multiplication_x4s[0], // => same but: m-times 4 views into same array texture
			&geometry_shader_multiplication_x4s[1],
			&geometry_shader_multiplication_x4s[2],
			&geometry_shader_multiplication_with_gcull_x4s[0], // => ...in addition culling in geometry shader
			&geometry_shader_multiplication_with_gcull_x4s[1],
			&geometry_shader_multiplication_with_gcull_x4s[2],
			&geometry_shader_multiplication_x4[0], // => same but: m-times 4 views into separate array textures
			&geometry_shader_multiplication_x4[1],
			&geometry_shader_multiplication_x4[2],
			&geometry_shader_multiplication_with_gcull_x4[0], // => ...in addition culling in geometry shader
			&geometry_shader_multiplication_with_gcull_x4[1],
			&geometry_shader_multiplication_with_gcull_x4[2],
			//// --------- NEW from 29.01.2020 onwards END ---------
			//// --------- Geometry Shader Multiplication techniques END ---------

			//// --------- ATTENTION END ---------
			
			//// --------- Geometry Shader Instancing techniques BEGIN ---------
			////
			//// In theory, Geometry Shader Instancing sounds pretty well.
			//// => Invoke the geometry shader V-times and the GPU will schedule everything perfectly, right?
			////   => WROOOONG... at least for the plain "rp311"-case.
			////
			//// However, if we apply the geometry shader triangle culling optimization and the _x4 optimization,
			//// we get strictly (to be verified!!) better performance than the Sorbier_..._x4 technique.
			//// Also, it seems like it should be that way because geometry shader instancing should actually
			//// be more optimal than Sorbier, because:
			////  - No need for calculating that translation matrix and translating every vertex' xy-positions separately
			////    ("separately" because of different .w coordinates of all the 3 vertices, i.e. additional calculations per vertex)
			////  - No need for manual culling in fragment shader (expensive discard) because culling is performed by the hardware
			////    (The viewport is specified, which for Sorbier it is not, so the hardware can perform fixed-function culling in clip space)
			//// 
			//&geom_instancing_aka_rp311[0], // => Call the geometry shader multiple times, i.e. once per V[]
			//&geom_instancing_aka_rp311[1],
			//&geom_instancing_aka_rp311[2],
			//// --------- NEW from 28/29.01.2020 onwards BEGIN ---------
			&geom_instancing_with_gcull[0], // => + additional culling in the geometry shader
			&geom_instancing_with_gcull[1],
			&geom_instancing_with_gcull[2],
			&geom_instancing_x4s[0], // => same but: m-times 4 views into same array texture
			&geom_instancing_x4s[1],							
			&geom_instancing_x4s[2],
			&geom_instancing_with_gcull_x4s[0], // => ...in addition culling in geometry shader
			&geom_instancing_with_gcull_x4s[1],
			&geom_instancing_with_gcull_x4s[2],
			&geom_instancing_x4[0], // => same but: m-times 4 views into separate array textures
			&geom_instancing_x4[1],
			&geom_instancing_x4[2],
			&geom_instancing_with_gcull_x4[0], // => ...in addition culling in geometry shader
			&geom_instancing_with_gcull_x4[1],
			&geom_instancing_with_gcull_x4[2],
			//// --------- NEW from 28/29.01.2020 onwards END ---------
			////
			//// --------- Geometry Shader Instancing techniques END ---------

			//// --------- instancing-based techniques BEGIN ---------
			//&instancing_planes_into_gtex[0],
			//&instancing_planes_into_gtex[1],
			//&instancing_planes_into_gtex[2],

			//&instancing_into_layers_aka_rp301[0],
			//&instancing_into_layers_aka_rp301[1],
			//&instancing_into_layers_aka_rp301[2],
			//&instancing_into_layers_passthrough_aka_rp301a[0],
			//&instancing_into_layers_passthrough_aka_rp301a[1],
			//&instancing_into_layers_passthrough_aka_rp301a[2],
			// --------- instancing-based techniques END ---------

			//// --------- multi-draw techniques BEGIN ---------
			////
			//// What's the point of multi-draw methods, really?
			//// I think, it's only to reduce the number of draw calls, right? Don't know what else it should be.
			//// The point is that you submit ONE instead of V[].size() draw calls and use gl_DrawID in the vertex shader to distinguish the views.
			////
			//// Can this really lead to performance improvements?
			//// Hmm, no one knows. Must be tested.
			//// In my imagination, this should not be any faster than, e.g. geometry shader instancing. I would go as far as to
			//// say that if anything, it could be slightly worse in the case of high vertex shader load, because maybe the GPU
			//// does not know that all vertex shaders are doing the same (or actually, they really aren't doing the same, but they
			//// are performing slightly different work work, which is useless (=> just setting variables to different gl_DrawIDs)).
			////
			//// Indeed, ONE of the two optimization (which there are triangle culling in geometry shader, and _x4) cuts no ice.
			//// Triangle culling in .geom DOES improve performance (probably by roughly the same factor as in other techniques),
			//// but _x4 does NOT improve performance.
			//// So, why is that? Something can't be parallelized which, for other techniques, can.
			//// ^ Maybe it is the dependance to gl_DrawID in the vertex shader which prevents ...what exactly? => proper parallelization?
			////
			//// Maybe one could state, as a guideline, that parallelizing as late as possible in the pipeline is favourable.
			//// (That would mean, for our pipelines, parallelize in the geometry shader! ...at least, this should hold true for high vertex loads)
			////
			//// The medium doesn't appear to make a difference here, i.e. doesn't matter if we use giant textures or texture arrays.
			//// 
			//&multidraw_into_gianttex_singlepass[0],
			//&multidraw_into_gianttex_singlepass[1],
			//&multidraw_into_gianttex_singlepass[2],
			//// --------- NEW from 30.01.2020 onwards BEGIN ---------
			&multidraw_into_gianttex_singlepass_with_gcull[0],
			&multidraw_into_gianttex_singlepass_with_gcull[1],
			&multidraw_into_gianttex_singlepass_with_gcull[2],
			&multidraw_into_gianttex_singlepass_x4[0],
			&multidraw_into_gianttex_singlepass_x4[1],
			&multidraw_into_gianttex_singlepass_x4[2],
			&multidraw_into_gianttex_singlepass_with_gcull_x4[0],
			&multidraw_into_gianttex_singlepass_with_gcull_x4[1],
			&multidraw_into_gianttex_singlepass_with_gcull_x4[2],
			//// --------- NEW from 30.01.2020 onwards END ---------
			
			//&multidraw_into_gianttex_singlepass_passthrough_aka_rp401[0],
			//&multidraw_into_gianttex_singlepass_passthrough_aka_rp401[1],
			//&multidraw_into_gianttex_singlepass_passthrough_aka_rp401[2],

			//&multidraw_into_layers_aka_rp312[0],
			//&multidraw_into_layers_aka_rp312[1],
			//&multidraw_into_layers_aka_rp312[2],
			//// --------- NEW from 31.01.2020 onwards BEGIN ---------
			&multidraw_into_layers_with_gcull[0],
			&multidraw_into_layers_with_gcull[1],
			&multidraw_into_layers_with_gcull[2],
			&multidraw_into_layers_x4s[0],
			&multidraw_into_layers_x4s[1],
			&multidraw_into_layers_x4s[2],
			&multidraw_into_layers_with_gcull_x4s[0],
			&multidraw_into_layers_with_gcull_x4s[1],
			&multidraw_into_layers_with_gcull_x4s[2],
			&multidraw_into_layers_x4[0],
			&multidraw_into_layers_x4[1],
			&multidraw_into_layers_x4[2],
			&multidraw_into_layers_with_gcull_x4[0],
			&multidraw_into_layers_with_gcull_x4[1],
			&multidraw_into_layers_with_gcull_x4[2],
			//// --------- NEW from 31.01.2020 onwards END ---------
			
			//&multidraw_into_layers_passthrough_aka_rp312a[0],
			//&multidraw_into_layers_passthrough_aka_rp312a[1],
			//&multidraw_into_layers_passthrough_aka_rp312a[2],

			//&multidraw_indirect_into_layers[0],
			//&multidraw_indirect_into_layers[1],
			//&multidraw_indirect_into_layers[2],
			//// --------- NEW from 31.01.2020 onwards BEGIN ---------
			&multidraw_indirect_into_layers_with_gcull[0],
			&multidraw_indirect_into_layers_with_gcull[1],
			&multidraw_indirect_into_layers_with_gcull[2],
			//&multidraw_indirect_into_layers_x4s[0], // Doesn't work => limited number of concurrent views not yet supported for MULTIDRAW_INDIRECT
			//&multidraw_indirect_into_layers_x4s[1],
			//&multidraw_indirect_into_layers_x4s[2],
			//&multidraw_indirect_into_layers_with_gcull_x4s[0],
			//&multidraw_indirect_into_layers_with_gcull_x4s[1],
			//&multidraw_indirect_into_layers_with_gcull_x4s[2],
			//// --------- NEW from 31.01.2020 onwards END ---------
			
			//&multidraw_indirect_into_layers_passthrough_aka_rp313[0],
			//&multidraw_indirect_into_layers_passthrough_aka_rp313[1],
			//&multidraw_indirect_into_layers_passthrough_aka_rp313[2],
			//// --------- multi-draw techniques END ---------
			
			//// --------- OVR-based techniques BEGIN ---------
			//&ovr_multiview_aka_rp501[0],
			//&ovr_multiview_aka_rp501[1],
			//&ovr_multiview_aka_rp501[2],
			//&ovr_multiview_4xN[0],
			//&ovr_multiview_4xN[1],
			//&ovr_multiview_4xN[2],
			//&ovr_multiview_2xN[0],
			//&ovr_multiview_2xN[1],
			//&ovr_multiview_2xN[2],
			//&ovr_multiview_into_gianttex_with_planes_4xN[0],
			//&ovr_multiview_into_gianttex_with_planes_4xN[1],
			//&ovr_multiview_into_gianttex_with_planes_4xN[2],
			//// --------- OVR-based techniques END ---------
			
			// !! ATTENTION: The following techniques show mediocre performance                    !!
			// !!            numbers and are, therefore, not/no longer included in regular tests.  !!
			// !!            They still work, though.                                              !!
			////&sorbier_cull_in_frag[0],
			////&sorbier_cull_in_frag[1],
			////&sorbier_cull_in_frag[2],
			////&sorbier_clip_planes[0],
			////&sorbier_clip_planes[1],
			////&sorbier_clip_planes[2],
			////&sorbier_cull_in_geom_clip_planes[0],
			////&sorbier_cull_in_geom_clip_planes[1],
			////&sorbier_cull_in_geom_clip_planes[2],
			////&sorbier_cull_in_both_x2_mpass[0],
			////&sorbier_cull_in_both_x2_mpass[1],
			////&sorbier_cull_in_both_x2_mpass[2],
			////&sorbier_cull_in_both_x6_mpass[0],
			////&sorbier_cull_in_both_x6_mpass[1],
			////&sorbier_cull_in_both_x6_mpass[2],
			////&sorbier_cull_in_both_x8_mpass[0],
			////&sorbier_cull_in_both_x8_mpass[1],
			////&sorbier_cull_in_both_x8_mpass[2],
			////&geom_instancing_with_plane_clipping[0], // useless plane clipping
			////&geom_instancing_with_plane_clipping[1],
			////&geom_instancing_with_plane_clipping[2],
			////&geom_instancing_with_gcull_and_plane_clipping[0], // useless plane clipping
			////&geom_instancing_with_gcull_and_plane_clipping[1],
			////&geom_instancing_with_gcull_and_plane_clipping[2],
			////&geom_instancing_with_plane_clipping_x4s[0], // useless plane clipping
			////&geom_instancing_with_plane_clipping_x4s[1],
			////&geom_instancing_with_plane_clipping_x4s[2],
			////&geom_instancing_with_gcull_and_plane_clipping_x4s[0], // useless plane clipping
			////&geom_instancing_with_gcull_and_plane_clipping_x4s[1],
			////&geom_instancing_with_gcull_and_plane_clipping_x4s[2],
			////&geom_instancing_with_plane_clipping_x4[0], // useless plane clipping
			////&geom_instancing_with_plane_clipping_x4[1],
			////&geom_instancing_with_plane_clipping_x4[2],
			////&geom_instancing_with_gcull_and_plane_clipping_x4[0], // useless plane clipping
			////&geom_instancing_with_gcull_and_plane_clipping_x4[1],
			////&geom_instancing_with_gcull_and_plane_clipping_x4[2],
			////&ovr_multiview_into_gianttex_with_planes_2xN[0],
			////&ovr_multiview_into_gianttex_with_planes_2xN[1],
			////&ovr_multiview_into_gianttex_with_planes_2xN[2],
			////&ovr_multiview_into_gianttex_with_planes_6xN[0],
			////&ovr_multiview_into_gianttex_with_planes_6xN[1],
			////&ovr_multiview_into_gianttex_with_planes_6xN[2],

			// !! ATTENTION: Giant Index Buffer Techniques don't work anymore currently.             !!
			// !!            The problem is (amongst others?) that the index buffers are             !!
			// !!            not enlargened/multiplied in GenericTechnique.cpp.                      !!
			//&giant_idxbfr_interleaved_aka_rp302,	// soooo slooow
			//&giant_idxbfr_consecutive,			// also pretty slow, but not as bad as the above

			// !! ATTENTION: extension GL_EXT_multiview_tessellation_geometry_shader not supported   !!
			//&ovr_multiview_into_gianttex_via_viewport_4xN[0],
			//&ovr_multiview_into_gianttex_via_viewport_4xN[1],
			//&ovr_multiview_into_gianttex_via_viewport_4xN[2],

		};

#pragma region top pipes
		std::vector<RenderPipeline*> topPipes = 
		{
			//// #11 [30..32]
			&multipass_separate_textures_aka_rp001[0],
			&multipass_separate_textures_aka_rp001[1],
			//&multipass_separate_textures_aka_rp001[2],

			//// #12 [33..35]
			//&multipass_into_gianttex_aka_rp402[0],
			//&multipass_into_gianttex_aka_rp402[1],
			//&multipass_into_gianttex_aka_rp402[2],

			//// #01 [0..2]
			//&sorbier_cull_in_both_x4_mpass[0],
			//&sorbier_cull_in_both_x4_mpass[1],
			//&sorbier_cull_in_both_x4_mpass[2],

			//// #02 [3..5]
			//&sorbier_cull_in_both_with_bfcull_x4_mpass[0],
			//&sorbier_cull_in_both_with_bfcull_x4_mpass[1],
			//&sorbier_cull_in_both_with_bfcull_x4_mpass[2],
//
//#if !defined(GBUFFER)
//			// #03 [6..8]
//			&sorbier_cull_in_both[0],
//			&sorbier_cull_in_both[1],
//			&sorbier_cull_in_both[2],
//#endif

#if !defined(GBUFFER)
			// #04 [9..11]
			&geom_instancing_with_gcull[0], // => + additional culling in the geometry shader
			&geom_instancing_with_gcull[1],
			//&geom_instancing_with_gcull[2],
#endif

			//// #05 [12..14]
			&geom_instancing_with_gcull_x4[0], // => ...in addition culling in geometry shader
			&geom_instancing_with_gcull_x4[1],
			//&geom_instancing_with_gcull_x4[2],
//
//			// #06 [15..17]
//			&geom_instancing_with_gcull_and_bfc_x4[0],
//			&geom_instancing_with_gcull_and_bfc_x4[1],
//			&geom_instancing_with_gcull_and_bfc_x4[2],
//
//#if !defined(GBUFFER)
//			// #07 [18..20]
			&geometry_shader_multiplication_with_gcull[0], // => + additional culling in the geometry shader
			&geometry_shader_multiplication_with_gcull[1],
//			&geometry_shader_multiplication_with_gcull[2],
//#endif
//
//			// #08 [21..23]
			&geometry_shader_multiplication_with_gcull_x4[0], // => ...in addition culling in geometry shader
			&geometry_shader_multiplication_with_gcull_x4[1],
//			&geometry_shader_multiplication_with_gcull_x4[2],
//
//			// #09 [24..26]
//			&geometry_shader_multiplication_with_gcull_and_bfc_x4[0],
//			&geometry_shader_multiplication_with_gcull_and_bfc_x4[1],
//			&geometry_shader_multiplication_with_gcull_and_bfc_x4[2],
//			
//			// #10 [27..29]
			&ovr_multiview_4xN[0],
			&ovr_multiview_4xN[1],
//			&ovr_multiview_4xN[2],
//
//			// #13 [36..38]
//			&instancing_planes_into_gtex[0],
//			&instancing_planes_into_gtex[1],
//			&instancing_planes_into_gtex[2],
//
//			// #14 [39..41]
//			&multidraw_indirect_into_layers_with_gcull[0],
//			&multidraw_indirect_into_layers_with_gcull[1],
//			&multidraw_indirect_into_layers_with_gcull[2],
//
//#if !defined(GBUFFER)
//			// #15 [42..44]
//			&multidraw_indirect_into_layers_with_gcull_and_bfc[0],
//			&multidraw_indirect_into_layers_with_gcull_and_bfc[1],
//			&multidraw_indirect_into_layers_with_gcull_and_bfc[2],
//#endif
//
//			// #16 [45..47]
//			&multidraw_indirect_into_layers_passthrough_aka_rp313[0],
//			&multidraw_indirect_into_layers_passthrough_aka_rp313[1],
//			&multidraw_indirect_into_layers_passthrough_aka_rp313[2],
//
//			// #17 [48..50]
//			&instancing_into_layers_passthrough_aka_rp301a[0],
//			&instancing_into_layers_passthrough_aka_rp301a[1],
//			&instancing_into_layers_passthrough_aka_rp301a[2],
//
//			// #18 [51..53]
//			&instancing_into_layers_passthrough_x4[0],
//			&instancing_into_layers_passthrough_x4[1],
//			&instancing_into_layers_passthrough_x4[2],
//
//			// #19 [54..56]
//			&ovr_multiview_aka_rp501[0],
//			&ovr_multiview_aka_rp501[1],
//			&ovr_multiview_aka_rp501[2],
//
//			// #20 [57..59]
//			&ovr_multiview_2xN[0],
//			&ovr_multiview_2xN[1],
//			&ovr_multiview_2xN[2],
		};
#pragma endregion 
		// some convenience cleanup functions:
		//for (int i = topPipes.size() - 1; i > 0; i -= 3) { // remove all the [2]and[1]
		//	topPipes.erase(topPipes.begin() + i);
		//	topPipes.erase(topPipes.begin() + i - 1);
		//}
		//for (int i = topPipes.size() - 1; i > 0; i -= 3) { // remove all the [2]
		//	topPipes.erase(topPipes.begin() + i);
		//}
		//for (int i = topPipes.size() - 2; i > 0; i -= 3) { // remove all the [1]
		//	topPipes.erase(topPipes.begin() + i);
		//}
	
		std::vector<RenderPipeline*> pipelinesToUse;

		if (argc >= 9) {
			std::string pipeIndexStr = argv[8];
			int pipeIndex = std::atoi(pipeIndexStr.c_str());
			if (pipeIndex >= 0 && pipeIndex < topPipes.size()) {
				if (&topPipes[pipeIndex]->geometry_provider() != &geometryEverything) {
					topPipes[pipeIndex]->geometry_provider().initWithBakedData(baked_vertices, indices, sceneName);
				}
				pipelinesToUse.push_back(topPipes[pipeIndex]);
			}
			else {
				throw std::runtime_error("Invalid pipeline index");
			}
		}
		else {
			pipelinesToUse = topPipes;
		}
	
		if (std::end(pipelinesToUse) != std::find_if(std::begin(pipelinesToUse), std::end(pipelinesToUse), [&](RenderPipeline*& p)
		{
			return &p->geometry_provider() == &geometryCubified100cubesCached;
		})) {
			// Init GEOMETRY PROVIDERS with vertex and index data, GL-buffers etc. will be created internally:
			geometryCubifiedToBeCached100cubes.initWithBakedData(baked_vertices, indices, sceneName); // a HUNDRED cubes :O
		}
	
		if (std::end(pipelinesToUse) != std::find_if(std::begin(pipelinesToUse), std::end(pipelinesToUse), [&](RenderPipeline*& p)
		{
			return &p->geometry_provider() == &geometryCubified1000cubesCached;
		})) {
			// Init GEOMETRY PROVIDERS with vertex and index data, GL-buffers etc. will be created internally:
			geometryCubifiedToBeCached1000cubes.initWithBakedData(baked_vertices, indices, sceneName); // a THOUSAND cubes :O :O
		}
			
		app.run(pipelinesToUse, measurementType, /* Start with pipeline at index: */ 0, autoModeEnabled, autoModeNumMeasurements);
	//}
	//catch(std::runtime_error& re) {
	//	std::cout << re.what() << std::endl;
	//}
	return 0;
}
