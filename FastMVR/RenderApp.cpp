#include "RenderApp.h"
#include <stdexcept>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "RenderPipeline.h"
#include "RenderResources.h"
#include <iostream>
#include "Stopwatch.h"
#include "ViewTechniquePipeline.h"
#include "QuakeCamera.h"
#include "ResultsWriter.h"
#include "GeometryProviderCubified.h"
#include "ViewPosGenerator.h"
#include "GeometryProviderCache.h"

enum struct geometry_render_selection { visibility_detected, everything };

RenderApp::RenderApp(int initialWidth, int initialHeight, GeometryProviderBase& geometryProviderForRendering) 
	: width(initialWidth)
	, height(initialHeight)
	, resources(nullptr)
	, geometryForRendering{std::ref(geometryProviderForRendering)}
{	
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GL_MAJOR_VERSION, 4);
	glfwWindowHint(GL_MINOR_VERSION, 5);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	window = glfwCreateWindow(width, height, "MVR", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	//if (glewInit() != GLEW_OK)
	//{
	//	throw std::runtime_error("Glew is not ok!");
	//}
}

void RenderApp::setResources(RenderResourcesGL* newResources)
{
	resources = newResources;
}

void RenderApp::run(std::vector<RenderPipeline*> pipelines, measurement_type measurementType, size_t startIndex, bool automaticModeOn, int numberOfMeasurementsInAutomaticMode)
{
	if (!resources)
	{
		throw std::runtime_error("Please set resources!");
	}
	std::unordered_map<RenderPipeline*, ResultsWriter> resultWriters;

	// Initialize and build all upfront, also initialize the ResultWriters
	for (auto* pipe : pipelines) {
		pipe->init(resources);

		std::string pipename_prefix = "";
		if (LOG_AND_IGNORE_GL_ERROR(glIsEnabled(GL_REPRESENTATIVE_FRAGMENT_TEST_NV)) == GL_TRUE) {
			pipename_prefix = "[rf]";
		}
		
		resultWriters[pipe] = ResultsWriter(path_to_results_output_directory(), pipename_prefix + pipe->name());
		std::cout << "Created a ResultsWriter which will write the results into " << resultWriters[pipe].full_output_directory_path() << std::endl;
	}
	RenderPipeline* pipeline = pipelines[startIndex];

	// A program which marks the visible primitives in the SSBO:
	MarkVisiblePrimitives primitivesMarker(static_cast<RenderResourcesGL*>(resources));
	primitivesMarker.buildProgram();

	// A program which counts all the marked primitives in the SSBO:
	VisiblePrimitivesCounter markedCounter(static_cast<RenderResourcesGL*>(resources));
	markedCounter.buildProgram();

	// A program which renders the primitives to screen which have been determined to be visible:
	RenderToScreen showtime(width, height, static_cast<RenderResourcesGL*>(resources), geometryForRendering);
	showtime.buildProgram();

	// A camera to navigate in the scene
	e186::QuakeCamera camera(window);
	camera.SetPerspectiveProjection(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

	double startTime = glfwGetTime();
	double lastTime = startTime;
	double deltaTime = 0.0;
	static const double writeTimeInterval = 10.0;

	int frameCounter = 0;
	const int numberOfSkipFrames = 0;
	std::optional<int> skipUntilFrame = frameCounter + numberOfSkipFrames; // skip the first frame in any case
	bool accumAllResults = false;
	int seletedResultTexId = 0;
	bool writeResults = false;
	bool paused = false;
	geometry_render_selection whichGeometryToRender = geometry_render_selection::visibility_detected;
	double nextWriteTime = glfwGetTime() + writeTimeInterval;
	auto Esc = KeyHelper(GLFW_KEY_ESCAPE, window);
	auto F1 = KeyHelper(GLFW_KEY_F1, window).set_went_down_handler([&]() {
		whichGeometryToRender = geometry_render_selection::visibility_detected;
		std::cout << "\nRendering the geometry which has been selected through visibility detection. (Change with [F2])\n" << std::endl;
	});
	auto F2 = KeyHelper(GLFW_KEY_F2, window).set_went_down_handler([&]() {
		whichGeometryToRender = geometry_render_selection::everything;
		std::cout << "\nRendering the entire scene. (Change with [F1])\n" << std::endl;
	});
	auto F3 = KeyHelper(GLFW_KEY_F3, window).set_went_down_handler([&]() {
		for (auto& g : additionalGeometryProviders) {
			g.get().setDebugModeEnabled(true);
		}
		std::cout << "\nDebug-Mode for all geometry providers activated. (Change with [F4])\n" << std::endl;
	});
	auto F4 = KeyHelper(GLFW_KEY_F4, window).set_went_down_handler([&]() {
		for (auto& g : additionalGeometryProviders) {
			g.get().setDebugModeEnabled(false);
		}
		std::cout << "\nDebug-Mode for all geometry providers disabled. (Change with [F3])\n" << std::endl;
	});
	auto F7 = KeyHelper(GLFW_KEY_F7, window).set_went_down_handler([&]() {
		showtime.setForcePrimitiveIdMappingOff(true);
		std::cout << "\nPrimitiveId-Mapping in screen renderer forced off. (Change with [F8])\n" << std::endl;
	});
	auto F8 = KeyHelper(GLFW_KEY_F8, window).set_went_down_handler([&]() {
		showtime.setForcePrimitiveIdMappingOff(false);
		std::cout << "\nPrimitiveId-Mapping in screen renderer NOT forced off/on depending on geometry provider. (Change with [F7])\n" << std::endl;
	});
	
	auto Num0 = KeyHelper(GLFW_KEY_KP_0, window).set_went_down_handler([&]() {
		showtime.set_geometry_provider(geometryForRendering);
		std::cout << "\nActivated the original geometry provider with the screen-renderer. (Change with [Num1], [Num2], [Num3])\n" << std::endl;
	});
	auto Num1 = KeyHelper(GLFW_KEY_KP_1, window).set_went_down_handler([&]() {
		if (additionalGeometryProviders.size() <= 0) { return; }
		showtime.set_geometry_provider(additionalGeometryProviders[0]);
		std::cout << "\nActivated additionalGeometryProvider[0] with the screen-renderer. (Change with [Num0], [Num2], [Num3])\n" << std::endl;
	});
	auto Num2 = KeyHelper(GLFW_KEY_KP_2, window).set_went_down_handler([&]() {
		if (additionalGeometryProviders.size() <= 1) { return; }
		showtime.set_geometry_provider(additionalGeometryProviders[1]);
		std::cout << "\nActivated additionalGeometryProvider[1] with the screen-renderer. (Change with [Num0], [Num1], [Num3])\n" << std::endl;
	});
	auto Num3 = KeyHelper(GLFW_KEY_KP_3, window).set_went_down_handler([&]() {
		if (additionalGeometryProviders.size() <= 2) { return; }
		showtime.set_geometry_provider(additionalGeometryProviders[2]);
		std::cout << "\nActivated additionalGeometryProvider[2] with the screen-renderer. (Change with [Num0], [Num1], [Num2])\n" << std::endl;
	});
	
	auto LShift = KeyHelper(GLFW_KEY_LEFT_SHIFT, window);
	auto LCtrl = KeyHelper(GLFW_KEY_LEFT_CONTROL, window);

	auto C = KeyHelper('C', window).set_went_down_handler([&]() {
		static glm::mat4 originalProjM = camera.projection_matrix();
		if (LShift.is_down()) {
			camera.set_projection_matrix(originalProjM);
		}
		else {
			camera.update_from_matrix(glm::inverse(pipeline->get_view_matrix(seletedResultTexId)));
			camera.set_projection_matrix(pipeline->get_proj_matrix(seletedResultTexId));
			camera.align_to_rotation_matrix();
		}
	});

	
	auto LArr = KeyHelper(GLFW_KEY_LEFT, window).set_went_down_handler([&]() {
		if (LShift.is_down() && LCtrl.is_down()) {
			for (auto& g : additionalGeometryProviders) {
				g.get().setDebugRenderEverything(false);
				g.get().setDebugIndex(g.get().getDebugIndex() - 1);
			}
		}
		else {
			seletedResultTexId = glm::clamp(seletedResultTexId - 1, 0, pipeline->get_view_matrices_count() - 1); accumAllResults = false;
		}
		C.m_went_down_handler();
	});
	auto RArr = KeyHelper(GLFW_KEY_RIGHT, window).set_went_down_handler([&]() {
		if (LShift.is_down() && LCtrl.is_down()) {
			for (auto& g : additionalGeometryProviders) {
				g.get().setDebugRenderEverything(false);
				g.get().setDebugIndex(g.get().getDebugIndex() + 1);
			}
		}
		else {
			seletedResultTexId = glm::clamp(seletedResultTexId + 1, 0, pipeline->get_view_matrices_count() - 1); accumAllResults = false;
		}
		C.m_went_down_handler();
	});
	auto UArr = KeyHelper(GLFW_KEY_UP, window).set_went_down_handler([&]() {
		if (!pipeline->activate_next_set()) {
			std::cout << "This is the last set. Can not switch to next set." << std::endl;
		}
		C.m_went_down_handler();
	});
	auto DArr = KeyHelper(GLFW_KEY_DOWN, window).set_went_down_handler([&]() {
		if (!pipeline->activate_prev_set()) {
			std::cout << "This is the first set. Can not switch to prev set." << std::endl;
		}
		C.m_went_down_handler();
	});
	auto Enter = KeyHelper(GLFW_KEY_ENTER, window).set_went_down_handler([&]() {
		accumAllResults = true;
		if (LShift.is_down() && LCtrl.is_down()) {
			for (auto& g : additionalGeometryProviders) {
				g.get().setDebugRenderEverything(true);
			}
		}
	});
	auto Space = KeyHelper(GLFW_KEY_SPACE, window).set_went_down_handler([&]() {
		writeResults = !writeResults;
		if (writeResults) { // => just enabled
			nextWriteTime = glfwGetTime() + writeTimeInterval;
			skipUntilFrame = frameCounter + numberOfSkipFrames;
		}
		else { // => just disabled
			resultWriters[pipeline].ResetBuffer();
		}
	});
	auto P = KeyHelper('P', window).set_went_down_handler([&]() { paused = !paused; });
	auto I = KeyHelper('I', window).set_went_down_handler([&]() {
		// Print info about all the available pipelines:
		std::cout << "\nPipelines:\n";
		for (int i=0; i < pipelines.size(); ++i) {
			if (pipelines[i] == pipeline) {
				std::cout << "[" << pipelines[i]->name() << "]";
			}
			else {
				std::cout << pipelines[i]->name();
			}
			if (i < pipelines.size() - 1) {
				std::cout << ", ";
			}
		}
		std::cout << "\n\n";
		std::cout << "Camera position: " << e186::vector_to_string(camera.GetPosition()) << "\n\n";
	});
	auto Home = KeyHelper(GLFW_KEY_HOME, window).set_went_down_handler([&]() { pipeline = pipelines[0]; std::cout << "\nActivated pipeline '" << pipeline->name() << "'\n\n"; });
	auto End = KeyHelper(GLFW_KEY_END, window).set_went_down_handler([&]() { pipeline = pipelines[pipelines.size() - 1]; std::cout << "\nActivated pipeline '" << pipeline->name() << "'\n\n"; });
	auto PgUp = KeyHelper(GLFW_KEY_PAGE_UP, window).set_went_down_handler([&]()	{
		if (writeResults) { // If the results are currently being written => finish the current pipeline before switching to the next
			resultWriters[pipeline].ResetBuffer();
			nextWriteTime = glfwGetTime() + writeTimeInterval; // Continue writing in 2 sec
			skipUntilFrame = frameCounter + numberOfSkipFrames;
		}
		pipeline = pipelines[std::min(static_cast<int>(pipelines.size() - 1), static_cast<int>(std::distance(std::begin(pipelines), std::find(std::begin(pipelines), std::end(pipelines), pipeline)) + 1))];
		std::cout << "\nActivated pipeline '" << pipeline->name() << "'\n\n";
	});
	auto PgDn = KeyHelper(GLFW_KEY_PAGE_DOWN, window).set_went_down_handler([&]() {
		if (writeResults) { // If the results are currently being written => finish the current pipeline before switching to the next
			resultWriters[pipeline].ResetBuffer();
			nextWriteTime = glfwGetTime() + writeTimeInterval; // Continue writing in 2 sec
			skipUntilFrame = frameCounter + numberOfSkipFrames;
		}
		pipeline = pipelines[std::max(0, static_cast<int>(std::distance(std::begin(pipelines), std::find(std::begin(pipelines), std::end(pipelines), pipeline)) - 1))];
		std::cout << "\nActivated pipeline '" << pipeline->name() << "'\n\n";
	});

	glEnable(GL_CULL_FACE);
	auto B = KeyHelper('B', window).set_went_down_handler([]() {
		static bool bfcEnabled = false;
		bfcEnabled = !bfcEnabled;
		if (bfcEnabled) {
			glEnable(GL_CULL_FACE);
			std::cout << "glEnable(GL_CULL_FACE);\n";
		}
		else {
			glDisable(GL_CULL_FACE);
			std::cout << "glDisable(GL_CULL_FACE);\n";
		}
	});

	bool dumpThisFrame = false;
	auto F = KeyHelper('F', window).set_went_down_handler([&]() 	{
		if (LShift.is_down()) {
			dumpThisFrame = true;
		}
	});

	int autoModeFrameCounter = 0;
	double autoAniLastAniTime = glfwGetTime();
	if (automaticModeOn) {
		writeResults = true;
	}

	GLuint64 queryStartTime, queryStopTime;
	GLuint queryID[2];
	if (measurementType == measurement_type::gpu_time) {
		// generate two queries
		glGenQueries(2, queryID);
	}

	ViewPosGenerator vpgen{false}; // true = enabled, false = disabled

	auto writeResultsFu = [&]() {
		resultWriters[pipeline].WriteBufferToAutomaticallyNamedFile();
		nextWriteTime = glfwGetTime() + writeTimeInterval;
	};

	C.m_went_down_handler();
	
	while (!glfwWindowShouldClose(window))
	{
		if (!automaticModeOn) {
			// Handle the input, which also runs key handlers
			Esc.Tick();
			F1.Tick();
			F2.Tick();
			F3.Tick();
			F4.Tick();
			F7.Tick();
			F8.Tick();
			Num0.Tick();
			Num1.Tick();
			Num2.Tick();
			Num3.Tick();
			LShift.Tick();
			LCtrl.Tick();
			LArr.Tick();
			RArr.Tick();
			UArr.Tick();
			DArr.Tick();
			Enter.Tick();
			Space.Tick();
			P.Tick();
			I.Tick();
			Home.Tick();
			End.Tick();
			PgUp.Tick();
			PgDn.Tick();
			C.Tick();
			F.Tick();
			B.Tick();
		}
		else {
			double curTime = glfwGetTime();
			if (curTime - autoAniLastAniTime > 0.3) { // => ani!
				autoAniLastAniTime = curTime;
				static KeyHelper* direction = &RArr;
				if (seletedResultTexId <= 0) { direction = &RArr; }
				if (seletedResultTexId >= pipeline->get_view_matrices_count() - 1) { direction = &LArr; }
				direction->m_went_down_handler();
			}
			C.m_went_down_handler();
		}

#ifdef MEASURE_DRAWCALLS
		dynamic_cast<GeometryProviderCache*>(&pipeline->geometry_provider())->m_resultsWriter = &resultWriters[pipeline];
#endif

		// Get current frame's delta time and update the camera
		auto curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		camera.Update(deltaTime);

		// 1. Produce the visibility set
		auto measuredMilliseconds = std::numeric_limits<double>::quiet_NaN();
		if (!paused) {	
			// Let's get's ready...
			resources->sync();

			if (vpgen.IsEnabled()) {
				vpgen.NextViewPos();
				const auto pM = pipeline->get_eval_config().projectionMatrix; // FUUUU, never forgetti!
				pipeline->get_eval_config().sets_of_matrices[0][0] = pM * vpgen.MatrixForCurrentViewPos();
			}

			Stopwatch frame_stopwatch;
			frame_stopwatch.EnablePrintingToConsole(false);
			if (pipeline->get_technique_config().include_begin_in_measurement()) { // EIIIIITHER start measurements HERE, ....
				Stopwatch tmp{fmt::format("Frame time {}", frameCounter)};
				tmp.EnablePrintingToConsole(false);
				frame_stopwatch = std::move(tmp);
				assert(std::numeric_limits<double>::has_quiet_NaN);
				if (measurementType == measurement_type::cpu_time) {
					frame_stopwatch.EnablePrintingToConsole(frameCounter % 20 == 0);
				}
				else {
					// issue the first query
					// Records the time only after all previous 
					// commands have been completed
					glQueryCounter(queryID[0], GL_TIMESTAMP);
				}
			}
			
			pipeline->begin(); 
			resources->sync();

			if (!pipeline->get_technique_config().include_begin_in_measurement()) { // OOOOORRRR start measurements HERE.
				Stopwatch tmp{fmt::format("Frame time {}", frameCounter)};
				tmp.EnablePrintingToConsole(false);
				frame_stopwatch = std::move(tmp);
				assert(std::numeric_limits<double>::has_quiet_NaN);
				if (measurementType == measurement_type::cpu_time) {
					frame_stopwatch.EnablePrintingToConsole(frameCounter % 20 == 0);
				}
				else {
					// issue the first query
					// Records the time only after all previous 
					// commands have been completed
					glQueryCounter(queryID[0], GL_TIMESTAMP);
				}
			}

			// I think, it would be more fair to include begin() in the measurements, because
			// indirect drawing is performing quite a bit of work in begin(), namely uploading
			// the indirects buffer and possibly also the primitive-id buffer.
			// For the other pipelines, all the equivalent tasks are performed in produce().
			pipeline->produce();

			if (measurementType == measurement_type::cpu_time) {
				resources->sync(); // Important to call before the measurement!
				
				auto elapsed = frame_stopwatch.Measure();
				measuredMilliseconds = std::chrono::duration<double, std::milli>(elapsed).count();
			}
			else {
				// issue the second query
				// records the time when the sequence of OpenGL 
				// commands has been fully executed
				glQueryCounter(queryID[1], GL_TIMESTAMP);

				// wait until the results are available
				GLint stopTimerAvailable = 0;
				while (!stopTimerAvailable) {
				    glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
				}
				
				// get query results
				glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &queryStartTime);
				glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &queryStopTime);

				measuredMilliseconds = (queryStopTime - queryStartTime) / 1000000.0;
				
				if (frameCounter % 20 == 0) {
					static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(std_output_handle, 0x02); 
					std::cout << "GPU-Query result time:      ";
					SetConsoleTextAttribute(std_output_handle, 0x0A); 
					printf("%lf ms", measuredMilliseconds);
					std::cout << std::endl;
					SetConsoleTextAttribute(std_output_handle, 0x0F); 
				}

				resources->sync(); // Probably don't needed... but doesn't hurt.
			}
			frame_stopwatch.Stop(); // Don't print anymore

			pipeline->end();
		}

		if (dumpThisFrame) {
			std::cout << "dumping results..." << std::endl;
			// 2. Dump the different ID m_textures to png files
			pipeline->dumpImages("../../../image_dump/");

			dumpThisFrame = false;
		}

		// 3. Merge the different ID m_textures
		if (geometry_render_selection::everything == whichGeometryToRender) {
			resources->clearPrimitiveEnabledBuffer(1u);
		}
		else { // => ENABLE ONLY VISIBLE TRIANGLES:

			resources->clearPrimitiveEnabledBuffer(0u);
			if (accumAllResults) { // all?...
				for (int i = 0; i < pipeline->get_view_matrices_count(); ++i) {
					// TODO-Q: Is this actually undefined behavior? Because there's no sync between the calls and no atomics are used.
					primitivesMarker.markAllFromTexture(
						pipeline->get_nth_result_texture_info(i), 
						pipeline->get_nth_result_offsets(i), 
						pipeline->get_nth_result_range(i));
				}
			}
			else { // ...or only one?
				primitivesMarker.markAllFromTexture(
					pipeline->get_nth_result_texture_info(seletedResultTexId), 
					pipeline->get_nth_result_offsets(seletedResultTexId), 
					pipeline->get_nth_result_range(seletedResultTexId)
				);
			}

			if (vpgen.IsEnabled()) {
				resources->sync();
				vpgen.LockBuffer();
				{
					markedCounter.computeCount();
					Stopwatch waitForCounter("atomic counter");
					resources->sync();
					vpgen.WaitBuffer(); // This sync does not seem to work...
				}
				auto numVisiblePrimitives = vpgen.ReadCounter(); // ...it gives the wrong result without the resources->sync()
			
				std::cout << "#visible primitives = " << numVisiblePrimitives << std::endl;
				if (vpgen.IsEnabled()) {
					if (numVisiblePrimitives > 3000) {
						vpgen.SaveCurrentViewPos("THE_views");
					}
				}
			
				vpgen.ResetCounter(); // Reset for next time
			}
			else {
				resources->sync(); // Sync before showtime
			}
		}

		// 4. Render only the selected triangles, because... why not?!
		showtime.begin();
#ifdef GBUFFER

#endif
		
#ifdef SHADOW_MAPPING
		GLuint textureview;
		auto prog_handle = resources->getProgramHandle(showtime.program_handle());
		static const GLint u_sampler_loc = [&](){ return GL_CALL(glGetUniformLocation(prog_handle, "u_sampler")); }();
		{	
			auto tex_info = pipeline->get_nth_depth_texture_info(seletedResultTexId);
			auto gl_handle = resources->getTextureHandle(tex_info.m_tex_name);
			glGenTextures(1, &textureview);
			glTextureView(textureview, GL_TEXTURE_2D,
			   gl_handle,
			   GL_DEPTH_COMPONENT32F,
			   0, 1,
			   tex_info.m_is_array_texture ? tex_info.m_layer : 0, 1);
			GL_CALL(glActiveTexture(GL_TEXTURE0));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, textureview));
			GL_CALL(glUniform1i(u_sampler_loc, 0));
		}

#ifdef GBUFFER
		GLuint textureviewNormals;
		static const GLint u_samplerNormals_loc = [&](){ return GL_CALL(glGetUniformLocation(prog_handle, "u_samplerNormals")); }();
		{
			auto tex_info = pipeline->get_nth_result_texture_info(seletedResultTexId, 1);
			auto gl_handle = resources->getTextureHandle(tex_info.m_tex_name);
			glGenTextures(1, &textureviewNormals);
			glTextureView(textureviewNormals, GL_TEXTURE_2D,
			   gl_handle,
			   GL_RGBA8,
			   0, 1,
			   tex_info.m_is_array_texture ? tex_info.m_layer : 0, 1);
			GL_CALL(glActiveTexture(GL_TEXTURE1));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, textureviewNormals));
			GL_CALL(glUniform1i(u_samplerNormals_loc, 1));
		}
		
		GLuint textureviewTriangleid;
		static const GLint u_samplerTriangleId_loc = [&](){ return GL_CALL(glGetUniformLocation(prog_handle, "u_samplerTriangleId")); }();
		{
			auto tex_info = pipeline->get_nth_result_texture_info(seletedResultTexId, 2);
			auto gl_handle = resources->getTextureHandle(tex_info.m_tex_name);
			glGenTextures(1, &textureviewTriangleid);
			glTextureView(textureviewTriangleid, GL_TEXTURE_2D,
			   gl_handle,
			   GL_R32UI,
			   0, 1,
			   tex_info.m_is_array_texture ? tex_info.m_layer : 0, 1);
			GL_CALL(glActiveTexture(GL_TEXTURE2));
			GL_CALL(glBindTexture(GL_TEXTURE_2D, textureviewTriangleid));
			GL_CALL(glUniform1i(u_samplerTriangleId_loc, 2));
		}
#endif
#endif
		
		showtime.render(camera.CalculateViewMatrix(), camera.projection_matrix());
		
#ifdef SHADOW_MAPPING
#ifdef GBUFFER
		glDeleteTextures(1, &textureviewTriangleid);
		glDeleteTextures(1, &textureviewNormals);
#endif
		glDeleteTextures(1, &textureview);
#endif

		// 5. Write results and move on to next view matrix/set in auto-mode
		if (!paused) {
			// Pass the result to the ResultsWriter and write to file once every ~2 seconds:
			// Format: frameNumber;duration
			if (writeResults) {
				if (!skipUntilFrame.has_value() || skipUntilFrame.value() < frameCounter) {
					// Add a measured result, ...
#if !defined(MEASURE_DRAWCALLS)
					resultWriters[pipeline].AddToBuffer(fmt::format("{};{}\n", frameCounter, measuredMilliseconds));
#endif
					if (curTime > nextWriteTime) {
						// ... and every once in a while, write it to file.
						writeResultsFu();
					}

					// Automatically move on to the next view position and to the next pipeline in automatic mode:
					if (automaticModeOn) {
						autoModeFrameCounter++;
						if (autoModeFrameCounter >= numberOfMeasurementsInAutomaticMode) {
							autoModeFrameCounter = 0;

							// 1. switch to next view position
							skipUntilFrame = frameCounter + numberOfSkipFrames;
							if (!pipeline->activate_next_set()) {
								// 2. Are there any pipelines left?
								writeResultsFu();
								
								auto awayFromEnd = std::distance(std::find(std::begin(pipelines), std::end(pipelines), pipeline), pipelines.end());
								if (awayFromEnd == 1) {
									glfwSetWindowShouldClose(window, GLFW_TRUE);
								}
								else {
									PgUp.m_went_down_handler();

									autoAniLastAniTime = glfwGetTime();
									seletedResultTexId = 0;
								}
							}
							
							
						}
					}

				}
				if (skipUntilFrame.has_value() && skipUntilFrame.value() < frameCounter) {
					skipUntilFrame = {};
				}
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (Esc.went_down_this_frame()) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		++frameCounter;

		lastTime = curTime;
	}

	writeResultsFu();
}

RenderApp::~RenderApp()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}