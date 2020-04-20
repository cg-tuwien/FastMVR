#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <stack>
#include <thread>
#include <cstdint>
#include <string>
#include <filesystem>
#include "GeometryProviderCubified.h"
#include "GLHelper.h"
#include "GeometryProviderEverything.h"
#include <fmt/format.h>
#include "Stopwatch.h"
#include <set>
#include <mutex>

GeometryProviderCubified::GeometryProviderCubified(int numCubes, bool clipTriangles)
	: GeometryProviderBase(std::string("cubi") + std::to_string(numCubes) + (clipTriangles ? "c" : "s"))
	, m_num_cubes{numCubes}
	, m_clip{clipTriangles}
{}

void GeometryProviderCubified::dumpConfiguration(
	std::string filename, 
	int numCubes, bool clipTriangles, 
	const std::vector<glm::vec3>& inCubifiedVertices, 
	const std::vector<uint32_t>& inCubifiedIndices, 
	const std::vector<uint32_t>& inPrimitiveMappingBuffer, 
	const std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& inRenderData)
{
	std::string dumpFile = filename + ".cubified." + std::to_string(numCubes) + "." + (clipTriangles ? "clipped" : "sloppy" ) + ".dump";
	std::filesystem::path dumpfile(dumpFile);

	uint32_t cubifiedVerticesCount = static_cast<uint32_t>(inCubifiedVertices.size());
	uint32_t cubifiedIndicesCount = static_cast<uint32_t>(inCubifiedIndices.size());
	uint32_t primitiveMappingCount = static_cast<uint32_t>(inPrimitiveMappingBuffer.size());
	uint32_t renderDataCount = static_cast<uint32_t>(inRenderData.size());

	std::ofstream df(dumpfile, std::ios_base::out | std::ios_base::binary);
	// dump sizes
	df.write(reinterpret_cast<char*>(&cubifiedVerticesCount),	sizeof(uint32_t));						//  1st write: count
	df.write(reinterpret_cast<char*>(&cubifiedIndicesCount),	sizeof(uint32_t));						//  2nd write: count
	df.write(reinterpret_cast<char*>(&primitiveMappingCount),	sizeof(uint32_t));						//  3nd write: count
	df.write(reinterpret_cast<char*>(&renderDataCount),			sizeof(uint32_t));						//  4th write: count

	// dump vertex- and index arrays
	const auto vtxSize = sizeof(inCubifiedVertices[0]) * cubifiedVerticesCount;
	const auto idxSize = sizeof(inCubifiedIndices[0])  * cubifiedIndicesCount;
	const auto priMapSize = sizeof(inPrimitiveMappingBuffer[0]) * primitiveMappingCount;
	const auto rendDataSize = sizeof(inRenderData[0]) * renderDataCount;
	df.write(reinterpret_cast<const char*>(inCubifiedVertices.data()), vtxSize);						//  5th write: all vertices
	df.write(reinterpret_cast<const char*>(inCubifiedIndices.data()),  idxSize);						//  6th write: all indices
	df.write(reinterpret_cast<const char*>(inPrimitiveMappingBuffer.data()), priMapSize);				//  7th write: all primitive mappings
	df.write(reinterpret_cast<const char*>(inRenderData.data()), rendDataSize);							//  8th write: all render data tuples

	//// dump the render data (sub-dump the primitive-id mapping vector)
	//for (uint32_t i = 0; i < renderDataCount; ++i) {													//  8th thing: loop
	//	auto& bounds = std::get<math::AABB>(inRenderData[i]);
	//	auto& renderData = std::get<GeometryRenderInfo>(inRenderData[i]);

	//	const auto aabbSize = sizeof(math::AABB);
	//	df.write(reinterpret_cast<const char*>(&bounds), aabbSize);										//  9th write: AABB
	//	const auto drawcallDataSize = sizeof(RenderResources::DrawElementsIndirectCommand);
	//	df.write(reinterpret_cast<const char*>(&renderData.m_drawcall_data), drawcallDataSize);			//  10th write: drawcall-struct

	//	const uint32_t primitiveIdMappingCount = static_cast<uint32_t>(renderData.m_primitive_mappings.size());
	//	df.write(reinterpret_cast<const char*>(&primitiveIdMappingCount), sizeof(uint32_t));			//  9th write: count
	//	const auto primMapSize = sizeof(renderData.m_primitive_mappings[0]) * primitiveIdMappingCount;
	//	df.write(reinterpret_cast<const char*>(renderData.m_primitive_mappings.data()), primMapSize);	// 10th write: all prim.mappings
	//}
}

bool GeometryProviderCubified::tryRestoreConfiguration(
	std::string filename, 
	int numCubes, bool clipTriangles, 
	std::vector<glm::vec3>& outCubifiedVertices, 
	std::vector<uint32_t>& outCubifiedIndices, 
	std::vector<uint32_t>& outPrimitiveMappingBuffer, 
	std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& outRenderData)
{
	std::string dumpFile = filename + ".cubified." + std::to_string(numCubes) + "." + (clipTriangles ? "clipped" : "sloppy" ) + ".dump";
	std::filesystem::path dumpfile(dumpFile);
	std::ifstream df(dumpfile, std::ios_base::in | std::ios_base::binary);
	if (!df.good()) { return false; }

	Stopwatch stopwatch("Restoring cubification data from file");

	uint32_t cubifiedVerticesCount;
	uint32_t cubifiedIndicesCount;
	uint32_t primitiveMappingCount;
	uint32_t renderDataCount;
	
	// restore sizes from file
	df.read(reinterpret_cast<char*>(&cubifiedVerticesCount), sizeof(uint32_t));					//  1st read: count
	df.read(reinterpret_cast<char*>(&cubifiedIndicesCount),	 sizeof(uint32_t));					//  2nd read: count
	df.read(reinterpret_cast<char*>(&primitiveMappingCount), sizeof(uint32_t));					//  3rd read: count
	df.read(reinterpret_cast<char*>(&renderDataCount),		 sizeof(uint32_t));					//  4th read: count
	
	outCubifiedVertices.resize(cubifiedVerticesCount);
	outCubifiedIndices.resize(cubifiedIndicesCount);
	outPrimitiveMappingBuffer.resize(primitiveMappingCount);
	outRenderData.resize(renderDataCount);
	// restore data in the big vectors from file
	const auto vtxSize = sizeof(outCubifiedVertices[0]) * cubifiedVerticesCount;
	const auto idxSize = sizeof(outCubifiedIndices[0])  * cubifiedIndicesCount;
	const auto priMapSize = sizeof(outPrimitiveMappingBuffer[0]) * primitiveMappingCount;
	const auto rendDataSize = sizeof(outRenderData[0]) * renderDataCount;
	df.read(reinterpret_cast<char*>(outCubifiedVertices.data()), vtxSize);						//  5th read: all vertices
	df.read(reinterpret_cast<char*>(outCubifiedIndices.data()),  idxSize);						//  6th read: all indices
	df.read(reinterpret_cast<char*>(outPrimitiveMappingBuffer.data()),  priMapSize);			//  7th read: all primitive mappings
	df.read(reinterpret_cast<char*>(outRenderData.data()),  rendDataSize);						//  8th read: all render data tuples

	//math:AABB aabbStorage;
	//// restore the render data (sub-restore the primitive-id mapping vector)
	//for (uint32_t i = 0; i < renderDataCount; ++i) {											//  6th thing: loop
	//	const auto aabbSize = sizeof(math::AABB);
	//	df.read(reinterpret_cast<char*>(&aabbStorage), aabbSize);								//  7th read: AABB
	//	
	//	auto& renderData = std::get<GeometryRenderData>(outRenderData.emplace_back(aabbStorage, GeometryRenderData{}));
	//	renderData.m_vertex_buffer = 0;					// will be set afterwards
	//	renderData.m_index_buffer = 0;					// will be set afterwards
	//	renderData.m_vao = 0;							// will be set afterwards
	//	renderData.m_primitive_id_mapping_buffer = 0;	// will be set afterwards

	//	const auto drawcallDataSize = sizeof(RenderResources::DrawElementsIndirectCommand);
	//	df.read(reinterpret_cast<char*>(&renderData.m_drawcall_data), drawcallDataSize);		//  8th read: drawcall-struct

	//	uint32_t primitiveIdMappingCount;
	//	df.read(reinterpret_cast<char*>(&primitiveIdMappingCount), sizeof(uint32_t));			//  9th read: count
	//	renderData.m_primitive_mappings.resize(primitiveIdMappingCount);
	//	auto primMapSize = sizeof(renderData.m_primitive_mappings[0]) * primitiveIdMappingCount;
	//	df.read(reinterpret_cast<char*>(renderData.m_primitive_mappings.data()), primMapSize);	// 10th read: all prim.mappings
	//}
	return true;
}

void fillRenderData(
	std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& renderDataVec, size_t beginIdx, size_t endIdx,
	bool clipTriangles,
	const std::vector<SceneVertex>& vertices,
	const std::vector<uint32_t>& indices,
	std::vector<glm::vec3>& cubifiedVertices, 
	std::vector<uint32_t>& cubifiedIndices,
	std::vector<uint32_t>& primitiveMappings)
{
	for (size_t rd = beginIdx; rd < endIdx; ++rd) {
		auto& tpl = renderDataVec[rd];
		auto& bounds = std::get<math::AABB>(tpl);
		auto& renderData = std::get<GeometryRenderInfo>(tpl);

		renderData.m_vertex_buffer = 0;					// will be set afterwards
		renderData.m_index_buffer = 0;					// will be set afterwards
		renderData.m_vao = 0;							// will be set afterwards
		renderData.m_primitive_id_mapping_buffer = 0;	// will be set afterwards

		// Gather the data for a draw call:
		// typedef  struct {
		//     GLuint  count;
		//     GLuint  instanceCount;
		//     GLuint  firstIndex;
		//     GLuint  baseVertex;
		//     GLuint  baseInstance;
		// } DrawElementsIndirectCommand;
		renderData.m_drawcall_data = {
			static_cast<GLuint>(cubifiedIndices.size()), // Just store the current value, calculate afterwards
			1u, // one instance by default
			// Offset firstIndex by the current length of the buffer: <-- THIS MUST BE MODIFIED OUTSIDE, when buffers are combined.
			static_cast<GLuint>(sizeof(uint32_t) * cubifiedIndices.size()), // Specifies a byte offset (cast to a pointer type) into the buffer bound to GL_ELEMENT_ARRAY_BUFFER to start reading indices from.
			0u, // Specifies a constant that should be added to each element of indices when chosing elements from the enabled vertex arrays.
			0u  // Specifies the base instance for use in fetching instanced vertex attributes.
		};

		// Build the mapping-buffer along the way => into the provided "primitiveMappings"-vector
		renderData.m_offset_into_primitive_id_mapping_buffer = static_cast<GLuint>(primitiveMappings.size()); // set the current offset (this will have to be modified outside when merging everything in a big buffer)

		// Iterate over ALL the input indices
		const auto numIndices = indices.size();
		for (size_t i = 0; i < numIndices; i += 3) { // 3 => triangles only
			math::Triangle tri{
				convert_to_float3(vertices[indices[i+0]].position), 
				convert_to_float3(vertices[indices[i+1]].position), 
				convert_to_float3(vertices[indices[i+2]].position)
			};
			// Is the triangle relevant for this particular cube/AABB?
			if (bounds.Intersects(tri)) {
				// If so => clip against the
				if (clipTriangles) {
					auto planes = get_planes_of_aabb(bounds, true);
					auto clippedTriangles = clip_against_frustum_planes(tri, planes);
					for (auto& ct : clippedTriangles) {
						auto vtxIndex = static_cast<uint32_t>(cubifiedVertices.size());
						cubifiedVertices.push_back(convert_to_vec3(ct.Vertex(0)));
						cubifiedVertices.push_back(convert_to_vec3(ct.Vertex(1)));
						cubifiedVertices.push_back(convert_to_vec3(ct.Vertex(2)));
						cubifiedIndices.push_back(vtxIndex);
						cubifiedIndices.push_back(vtxIndex + 1);
						cubifiedIndices.push_back(vtxIndex + 2);

						uint32_t originalPrimitiveId = i / 3;
						primitiveMappings.push_back(originalPrimitiveId);			
					}
				}
				else {
					auto vtxIndex = static_cast<uint32_t>(cubifiedVertices.size());
					cubifiedVertices.push_back(vertices[indices[i+0]].position);
					cubifiedVertices.push_back(vertices[indices[i+1]].position);
					cubifiedVertices.push_back(vertices[indices[i+2]].position);
					cubifiedIndices.push_back(vtxIndex);
					cubifiedIndices.push_back(vtxIndex + 1);
					cubifiedIndices.push_back(vtxIndex + 2);

					uint32_t originalPrimitiveId = i / 3;
					primitiveMappings.push_back(originalPrimitiveId);					
				}
			}
		}

		// Set the correct indices-count:
		renderData.m_drawcall_data.count = (static_cast<GLuint>(cubifiedIndices.size()) - renderData.m_drawcall_data.count);
	}
}

void GeometryProviderCubified::initWithBakedData(const std::vector<SceneVertex>& vertices, const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom)
{
	// Determine the number of cubes and their (uniform) dimensions
	std::vector<vec> float3s;
	for (auto& v : vertices) {
		float3s.emplace_back(v.position.x, v.position.y, v.position.z); // Transfer into a different format temporarily
	}
	const auto everything = math::AABB::MinimalEnclosingAABB(float3s.data(), float3s.size());


	const auto extent = everything.Size();
	assert(extent.x > 0.0f);
	assert(extent.y > 0.0f);
	assert(extent.z > 0.0f);
	// Divide the AABB equally:
	const auto s = std::cbrtf(static_cast<float>(m_num_cubes) / (extent.x * extent.y * extent.z));
	const auto ex = extent.x * s;
	const auto ey = extent.y * s;
	const auto ez = extent.z * s;
	const auto nx = static_cast<int>(ex) + 1;
	const auto ny = static_cast<int>(ey) + 1;
	const auto nz = static_cast<int>(ez) + 1;
	const auto sideLen = 1.0f / s;
	const auto cubeDim = float3(sideLen);

	// Center the cubes by shifting by the excess dimensions:
	auto centerOffset = float3{
		(sideLen * nx - extent.x) * 0.5f,
		(sideLen * ny - extent.y) * 0.5f,
		(sideLen * nz - extent.z) * 0.5f,
	};
	assert(centerOffset.x < sideLen * 0.5f);
	assert(centerOffset.y < sideLen * 0.5f);
	assert(centerOffset.z < sideLen * 0.5f);

	const unsigned int nThreads = std::max(std::thread::hardware_concurrency(), 1u);

	// Create the cubes:
	auto actualNumCubes = nx * ny * nz;
	m_render_data.reserve(actualNumCubes);
	std::cout << fmt::format("\nRequiring {} cubes ({}x{}x{}), geometry is{}being clipped.", actualNumCubes, nx, ny, nz, m_clip ? " " : " not ") << std::endl;

	std::vector<glm::vec3> cubifiedVertices;
	std::vector<uint32_t> cubifiedIndices;
	std::vector<uint32_t> primitiveIdMappings;
	if (tryRestoreConfiguration(filenameWhereItHasBeenLoadedFrom, actualNumCubes, m_clip, cubifiedVertices, cubifiedIndices, primitiveIdMappings, m_render_data)) {
		std::cout << fmt::format("\nCubification config for file {} restored from dump file!", filenameWhereItHasBeenLoadedFrom) << std::endl;
	}
	else {
		std::cout << fmt::format("\nGoing to cubify into {} cubes using {} thread(s).", actualNumCubes, nThreads) << std::endl;
		Stopwatch stopwatch("Cubification");
		
		for (auto x = decltype(nx){0}; x < nx; ++x) {
			auto offx = float3(sideLen * x, 0.f, 0.f);
			for (auto y = decltype(ny){0}; y < ny; ++y) {
				auto offy = float3(0.f, sideLen * y, 0.f);
				for (auto z = decltype(nz){0}; z < nz; ++z) {
					auto offz = float3(0.f, 0.f, sideLen * z);
					auto from = everything.minPoint - centerOffset + offx + offy + offz;
					auto to = from + cubeDim;
					auto bounds = math::AABB(from, to);
					m_render_data.emplace_back(bounds, GeometryRenderInfo{});
				}
			}
		}
		
		// Iterate over all the cubes, and gather the geometry they represent, parallelized in multiple threads

		const auto nRd = m_render_data.size();
		const auto nRdPerThread = (nRd % nThreads) == 0 ? (nRd / nThreads) : ((nRd / nThreads) + 1);
		auto nRemaining = nRd;
		std::vector<std::thread> threads;
		std::vector<std::tuple<size_t, size_t>> renderDataIndexBounds;
		std::vector<std::vector<glm::vec3>> tmpVertices;
		std::vector<std::vector<uint32_t>> tmpIndices;
		std::vector<std::vector<uint32_t>> tmpPrimMappings;
		threads.reserve(nThreads);
		tmpVertices.reserve(nThreads);
		tmpIndices.reserve(nThreads);
		tmpPrimMappings.reserve(nThreads);
		// Compute in parallel:
		for (size_t i = 0; i < nThreads; ++i) {
			auto nThreadRds = std::min(nRdPerThread, nRemaining);
			auto threadBeginIdx = i * nRdPerThread;
			auto threadEndIdx = threadBeginIdx + nThreadRds;
			renderDataIndexBounds.emplace_back(std::make_tuple(threadBeginIdx, threadEndIdx));
			tmpVertices.emplace_back();
			tmpIndices.emplace_back();
			tmpPrimMappings.emplace_back();
			//std::vector<std::tuple<math::AABB, GeometryRenderData>>& renderData, size_t beginIdx, size_t endIdx,
			//bool clipTriangles,
			//const std::vector<SceneVertex>& vertices,
			//const std::vector<uint32_t>& indices,
			//std::vector<glm::vec3>& cubifiedVertices, 
			//std::vector<uint32_t>& cubifiedIndices
			threads.emplace_back(fillRenderData,
				std::ref(m_render_data), threadBeginIdx, threadEndIdx,
				m_clip,
				std::cref(vertices),
				std::cref(indices),
				std::ref(tmpVertices[i]), 
				std::ref(tmpIndices[i]),
				std::ref(tmpPrimMappings[i]));
			nRemaining -= nThreadRds;
		}
		// Join and merge the data:
		std::vector<size_t> renderDataToRemove; // because they're empty
		for (size_t i = 0; i < nThreads; ++i) {
			threads[i].join();

			// Set the right byte offset for the draw calls, i.e. where to start reading indices from:
			const auto indicesShift = cubifiedIndices.size();
			const auto primMappingShift = primitiveIdMappings.size();
			auto rdBegin = std::get<0>(renderDataIndexBounds[i]);
			auto rdEnd = std::get<1>(renderDataIndexBounds[i]);
			for (auto rd = rdBegin; rd < rdEnd; ++rd) {
				auto& renderData = std::get<GeometryRenderInfo>(m_render_data[rd]);
				if (0 == renderData.m_drawcall_data.count) {
					renderDataToRemove.push_back(rd);
				}
				else {
					// Offset the following two even more to correctly set the offsets in the single, big, huge index and primitive-mapping buffers, respectively:
					renderData.m_drawcall_data.firstIndex += static_cast<GLuint>(sizeof(uint32_t) * indicesShift);
					renderData.m_offset_into_primitive_id_mapping_buffer += static_cast<GLuint>(primMappingShift);
				}
			}

			// Gotta make sure to modify the indices accordingly, when merging them all into a single vertex buffer:
			const auto nVerts = static_cast<uint32_t>(cubifiedVertices.size());
			cubifiedVertices.insert(std::end(cubifiedVertices), std::make_move_iterator(std::begin(tmpVertices[i])), std::make_move_iterator(std::end(tmpVertices[i])));
			for (auto ind : tmpIndices[i]) {
				cubifiedIndices.emplace_back(ind + nVerts);
			}
			primitiveIdMappings.insert(std::end(primitiveIdMappings), std::make_move_iterator(std::begin(tmpPrimMappings[i])), std::make_move_iterator(std::end(tmpPrimMappings[i])));
		}

		if (renderDataToRemove.size() > 0) {
			std::cout << fmt::format("Removing {} renderData entries, because they're empty.", renderDataToRemove.size()) << std::endl;
			for (auto x = renderDataToRemove.size(); x > 0; --x) {
				m_render_data.erase(m_render_data.begin() + renderDataToRemove[x - 1]);
			}
		}
		
		dumpConfiguration(filenameWhereItHasBeenLoadedFrom, actualNumCubes, m_clip, cubifiedVertices, cubifiedIndices, primitiveIdMappings, m_render_data);
	}
	
#pragma region create and fill GPU buffers
	GLuint vertexBufferHandle;
	GL_CALL(glGenBuffers(1, &vertexBufferHandle));
	GLuint vaoHandle;
	GL_CALL(glCreateVertexArrays(1, &vaoHandle));
	GLuint indexBufferHandle;
	GL_CALL(glGenBuffers(1, &indexBufferHandle));
	GLuint primMappingBufferHandle;
	GL_CALL(glGenBuffers(1, &primMappingBufferHandle));
	
	// Upload Data to the GPU:
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cubifiedVertices.size(), cubifiedVertices.data(), GL_STATIC_DRAW));

	GL_CALL(glBindVertexArray(vaoHandle));
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_POS, SceneVertex::SIZE_POS, SceneVertex::TYPE_POS, GL_FALSE, sizeof(glm::vec3), static_cast<GLvoid*>(0)));
#ifdef GBUFFER
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_NORM, SceneVertex::SIZE_NORM, SceneVertex::TYPE_NORM, GL_FALSE, sizeof(glm::vec3), static_cast<GLvoid*>(0)));
	GL_CALL(glVertexAttribPointer(SceneVertex::ID_TRIANGLEID, SceneVertex::SIZE_TRIANGLEID, SceneVertex::TYPE_TRIANGLEID, GL_FALSE, sizeof(glm::vec3), static_cast<GLvoid*>(0)));
#endif
	
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * cubifiedIndices.size(), cubifiedIndices.data(), GL_STATIC_DRAW));

	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, primMappingBufferHandle));
	GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * primitiveIdMappings.size(), primitiveIdMappings.data(), GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
	

	for (auto& tpl : m_render_data) {
		auto& bounds = std::get<math::AABB>(tpl);
		auto& renderData = std::get<GeometryRenderInfo>(tpl);

		renderData.m_vertex_buffer = vertexBufferHandle;
		renderData.m_index_buffer = indexBufferHandle;
		renderData.m_vao = vaoHandle;
		renderData.m_primitive_id_mapping_buffer = primMappingBufferHandle;
	}
#pragma endregion 
}

size_t GeometryProviderCubified::maxNumberOfPrimitiveMappingOffsets()
{
	return m_render_data.size();
}

GeometryProviderCubified::~GeometryProviderCubified()
{
	for (auto& tpl : m_render_data)
	{
		auto& renderData = std::get<GeometryRenderInfo>(tpl);
		auto& aabb = std::get<math::AABB>(tpl);

		if (renderData.m_primitive_id_mapping_buffer)
		{
			GL_CALL(glDeleteBuffers(1, &renderData.m_primitive_id_mapping_buffer));
		}
		if (renderData.m_vao)
		{
			GL_CALL(glDeleteVertexArrays(1, &renderData.m_vao));
		}
		if (renderData.m_index_buffer)
		{
			GL_CALL(glDeleteBuffers(1, &renderData.m_index_buffer));
		}
		if (renderData.m_vertex_buffer)
		{
			GL_CALL(glDeleteBuffers(1, &renderData.m_vertex_buffer));
		}

		break; // break after the first one, because the buffers are the same for all of them!
	}
}

void selectRenderDataIndices(
	const std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& renderDataVec, size_t beginIdx, size_t endIdx,
	const std::vector<std::array<math::Plane, 6>>& frustums,
	std::set<size_t>& outSelectedIndices, 
	std::mutex& insertMutex)
{
	size_t nFrust = frustums.size();
	for (size_t rd = beginIdx; rd < endIdx; ++rd) {
		auto& bounds = std::get<math::AABB>(renderDataVec[rd]);
		// Test each cube with every matrix:
		for (size_t f = 0; f < nFrust; ++f) {
			if (is_aabb_in_frustum(frustums[f], bounds)) {
				std::scoped_lock<std::mutex> guard(insertMutex);
				outSelectedIndices.insert(rd);
				break; // This AABB has been selected => no need to test with further frustums
			}
		}
	}
}

std::vector<GeometryRenderInfo> GeometryProviderCubified::getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices)
{
	Stopwatch stopwatch("select relevant geometry GeometryProviderCubified::getGeometryForMatrices");

	if (m_debug_mode_enabled) {
		if (m_debug_mode_render_everything) {
			std::vector<GeometryRenderInfo> everything;
			everything.reserve(m_render_data.size());
			for (auto& tpl : m_render_data) {
				everything.push_back(std::get<GeometryRenderInfo>(tpl));
			}
			return everything;
		}
		//std::cout << "WARNING: Debug-Mode enabled in GeometryProviderCubified::getGeometryForMatrices" << std::endl;
		size_t cubeIndexToReturn = glm::clamp(m_debug_index, static_cast<size_t>(0), m_render_data.size() - 1);
		return { std::get<GeometryRenderInfo>(m_render_data[cubeIndexToReturn]) };
	}

	std::vector<std::array<math::Plane, 6>> frustums;
	for (auto& M : viewProjMatrices) {
		frustums.push_back(convert_to_math_frustum_planes(gamedevs::ExtractPlanesGL(M, true)));
	}

	// Measure the "produce"-step of the pipeline.
	static int frameCounter = 0;
	Stopwatch frame_stopwatch("Selecting cubes for multiple matrices");
	frame_stopwatch.EnablePrintingToConsole(frameCounter % 100 == 0);
	frameCounter++;

	// Search in parallel:
	static const unsigned int nThreads = std::max(std::thread::hardware_concurrency(), 1u);
	const auto nRd = m_render_data.size();
	const auto nRdPerThread = (nRd % nThreads) == 0 ? (nRd / nThreads) : ((nRd / nThreads) + 1);
	auto nRemaining = nRd;
	std::vector<std::thread> threads;
	std::set<size_t> selectedIndices;
	static std::mutex indicesAccessMutex;
	threads.reserve(nThreads);
	// Compute in parallel:
	for (size_t i = 0; i < nThreads; ++i) {
		auto nThreadRds = std::min(nRdPerThread, nRemaining);
		auto threadBeginIdx = i * nRdPerThread;
		auto threadEndIdx = threadBeginIdx + nThreadRds;
		threads.emplace_back(selectRenderDataIndices,
			std::cref(m_render_data), threadBeginIdx, threadEndIdx,
			frustums,
			std::ref(selectedIndices),
			std::ref(indicesAccessMutex));
		nRemaining -= nThreadRds;
	}
	for (size_t i = 0; i < nThreads; ++i) {
		threads[i].join();
	}

	// Gather the result from the unique indices
	std::vector<GeometryRenderInfo> result;
	result.reserve(selectedIndices.size());
	for (auto i : selectedIndices) {
		result.push_back(std::get<GeometryRenderInfo>(m_render_data[i]));
	}
	stopwatch.Measure(fmt::format("selected {} cubes", result.size()));
	stopwatch.Stop();
	return result;
}
