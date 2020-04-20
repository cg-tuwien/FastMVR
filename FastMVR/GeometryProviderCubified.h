#pragma once
#include "GeometryProvider.h"
#define GL_SCENE
#include "Scene.h"
#undef GL_SCENE
#include "MathGeoLib.h"

class GeometryProviderCubified : public GeometryProviderBase
{
public:
	GeometryProviderCubified(int numCubes, bool clipTriangles);
	virtual ~GeometryProviderCubified();

	void initWithBakedData(const std::vector<SceneVertex>& vertices,const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom) override;
	bool requiresPrimitiveIdMapping() override { return true; }
	size_t maxNumberOfPrimitiveMappingOffsets() override;
	std::vector<GeometryRenderInfo> getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices) override;

	void dumpConfiguration(
		std::string filename, 
		int numCubes, bool clipTriangles, 
		const std::vector<glm::vec3>& inCubifiedVertices, 
		const std::vector<uint32_t>& inCubifiedIndices, 
		const std::vector<uint32_t>& inPrimitiveMappingBuffer, 
		const std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& inRenderData);
	bool tryRestoreConfiguration(
		std::string filename, 
		int numCubes, bool clipTriangles, 
		std::vector<glm::vec3>& outCubifiedVertices, 
		std::vector<uint32_t>& outCubifiedIndices,
		std::vector<uint32_t>& outPrimitiveMappingBuffer, 
		std::vector<std::tuple<math::AABB, GeometryRenderInfo>>& outRenderData);
	
private:
	int m_num_cubes;
	bool m_clip;
	std::vector<std::tuple<math::AABB, GeometryRenderInfo>> m_render_data;
};

