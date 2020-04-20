#pragma once
#include "GeometryProvider.h"
#define GL_SCENE
#include "Scene.h"
#undef GL_SCENE


class GeometryProviderEverything :
	public GeometryProviderBase
{
public:
	GeometryProviderEverything();
	virtual ~GeometryProviderEverything();

	void initWithBakedData(const std::vector<SceneVertex>& vertices,const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom) override;
	bool requiresPrimitiveIdMapping() override { return false; }
	std::vector<GeometryRenderInfo> getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices) override;

private:
	std::vector<uint32_t> m_stored_indices;
	GeometryRenderInfo m_render_data;
};

