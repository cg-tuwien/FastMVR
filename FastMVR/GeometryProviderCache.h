#pragma once
#include "GeometryProvider.h"
#include <unordered_map>
#include "hash_utils.h"
#include "Stopwatch.h"
#include "ResultsWriter.h"

/** A wrapper around a geometry provider that caches its results. */
class GeometryProviderCache :
	public GeometryProviderBase
{
public:
	GeometryProviderCache(GeometryProviderBase& providerToBeWrapped)
		: GeometryProviderBase("GeometryProviderCache")
		, m_wrappedProvider { &providerToBeWrapped }
	{ }
	
	~GeometryProviderCache() {};

	void initWithBakedData(const std::vector<SceneVertex>& vertices,const std::vector<uint32_t>& indices, std::string filenameWhereItHasBeenLoadedFrom) override
	{
		m_wrappedProvider->initWithBakedData(vertices, indices, filenameWhereItHasBeenLoadedFrom);
	}

	const std::string& name() override { return m_wrappedProvider->name(); }

	bool requiresPrimitiveIdMapping() override { return m_wrappedProvider->requiresPrimitiveIdMapping(); }

	size_t maxNumberOfPrimitiveMappingOffsets() override { return m_wrappedProvider->maxNumberOfPrimitiveMappingOffsets(); }
	
	std::vector<GeometryRenderInfo> getGeometryForMatrices(std::vector<glm::mat4> viewProjMatrices) override
	{
		//Stopwatch stopwatch("getGeometryForMatrices: query std::vector<GeometryRenderInfo> from cache");

		if (getDebugModelEnabled()) {
			return m_wrappedProvider->getGeometryForMatrices(viewProjMatrices);
		}

		auto h = std::hash<std::vector<glm::mat4>>{}(viewProjMatrices);
		auto it = m_cache.find(h);
		if (it != m_cache.end()) {
			return it->second;
		}
		m_cache[h] = m_wrappedProvider->getGeometryForMatrices(viewProjMatrices);
#ifdef MEASURE_DRAWCALLS
		static int frameCounter = 0;
		frameCounter++;
		m_resultsWriter->AddToBuffer(fmt::format("{};{}\n", frameCounter, m_cache[h].size()));
#endif
		return m_cache[h];
	}

	size_t getDebugIndex() override
	{
		return m_wrappedProvider->getDebugIndex();
	}

	bool getDebugModelEnabled() override
	{
		return m_wrappedProvider->getDebugModelEnabled();
	}

	void setDebugModeEnabled(bool enable) override
	{
		m_wrappedProvider->setDebugModeEnabled(enable);
	}

	void setDebugIndex(int index) override
	{
		m_wrappedProvider->setDebugIndex(index);
	}

	void setDebugRenderEverything(bool doIt) override
	{
		m_wrappedProvider->setDebugRenderEverything(doIt);
	}

	bool getDebugRenderEverything() override
	{
		return m_wrappedProvider->getDebugRenderEverything();
	}

#ifdef MEASURE_DRAWCALLS
	ResultsWriter* m_resultsWriter;
#endif
	
private:
	GeometryProviderBase* m_wrappedProvider;
	// map from hash to render data:
	std::unordered_map<std::size_t, std::vector<GeometryRenderInfo>> m_cache;
};

