#pragma once
#include "EvalConfig.h"
#include <memory>
#include "ViewTechniquePipeline.h"
#include "DepthTechniquePipeline.h"

class RenderPipeline;

template<typename T, typename S, typename ... V>
void collectPipelines(std::vector<RenderPipeline*>& pipelines, EvalConfig evconfig, GeometryProviderBase& geometry)
{
	pipelines.push_back(T::getPipeline(evconfig));
	collectPipelines<S, V...>(evconfig);
}

template<typename T>
void collectPipelines(std::vector<RenderPipeline*>& pipelines, EvalConfig evconfig, GeometryProviderBase& geometry)
{
	pipelines.push_back(T::getPipeline(evconfig, geometry));
}

// Depth caring
template
<
	DepthTechnique::DepthLayerSorting DLS,
	DepthTechnique::DepthLayerMedium DLM,
	DepthTechnique::DepthLayerChange DLC
>
class DepthRenderMode
{
public:
	static RenderPipeline* getPipeline(EvalConfig evconfig, GeometryProviderBase& geometry);
};

// Depth oblivious
template
	<
	ViewTechnique::MultiViewDuplication MVD,
	ViewTechnique::MultiViewMedium MVM,
	ViewTechnique::MultiViewChange MVC = ViewTechnique::MultiViewChange::NONE,
	ViewTechnique::MultiViewPassThrough MVPT = ViewTechnique::MultiViewPassThrough::DISABLED
	>
	class ViewRenderMode
{
public:
	static RenderPipeline* getPipeline(EvalConfig evconfig, GeometryProviderBase& geometry);
};