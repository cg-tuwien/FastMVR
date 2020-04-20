#include "RenderMode.h"
#include "RenderResources.h"
#include "Helper.h"
#include "GeometryProvider.h"

// View point techniques
#include "RP001.h"
#include "RP201.h"
#include "RP202.h"
#include "RP203.h"
#include "RP301.h"
#include "RP301a.h"
#include "RP302.h"
#include "RP311.h"
#include "RP312.h"
#include "RP312a.h"
#include "RP313.h"
#include "RP401.h"
#include "RP402.h"
#include "RP501.h"

// Depth layerin techniques
#include "RP701.h"

#define ADD_VIEW_TECHNIQUE(MD, MM, MC, MP, T)										\
RenderPipeline* ViewRenderMode														\
< MD, MM, MC, MP >																	\
::getPipeline(EvalConfig evconfig, GeometryProviderBase& geometry)													\
{																					\
	return new T(evconfig.resolution_x, evconfig.resolution_y, evconfig.sets_of_matrices[0], geometry);	\
}

#define ADD_DEPTH_TECHNIQUE(DS, DM, DC, T)																		\
RenderPipeline* DepthRenderMode																					\
< DS, DM, DC >																									\
::getPipeline(EvalConfig evconfig, GeometryProviderBase& geometry)																				\
{																												\
	return new T(evconfig.resolution_x, evconfig.resolution_y, evconfig.sets_of_matrices[0][0], evconfig.depth_ranges, geometry);	\
}

// @TODO: Experiment more with interesting combinations, don't be afraid to try new things!

using VT = ViewTechnique;

// Won't implement in GenericTechnique:
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIPASS, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::FRAMEBUFFER, VT::MultiViewPassThrough::DISABLED, RP201) // 1 Texture Array with 1 layer each for a framebuffer => makes no sense.
// Already implemented in GenericTechnique:
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIPASS, VT::MultiViewMedium::SEPARATE_TEXTURES, VT::MultiViewChange::FRAMEBUFFER, VT::MultiViewPassThrough::DISABLED, RP001)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIPASS, VT::MultiViewMedium::GIANT_TEXTURE, VT::MultiViewChange::VIEWPORT, VT::MultiViewPassThrough::DISABLED, RP402)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIDRAW, VT::MultiViewMedium::GIANT_TEXTURE, VT::MultiViewChange::VIEWPORT, VT::MultiViewPassThrough::ENABLED, RP401)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::GEOMETRY_SHADER_LOOP, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP202)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIPASS, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP203)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::GIANT_INDEX_BUFFER, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP302)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::GEOMETRY_SHADER_INSTANCING, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP311)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::INSTANCING, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP301)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::INSTANCING, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::ENABLED, RP301a)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIDRAW, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP312)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIDRAW, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::ENABLED, RP312a)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::MULTIDRAW_INDIRECT, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::ENABLED, RP312a)
ADD_VIEW_TECHNIQUE(VT::MultiViewDuplication::OVR_MULTIVIEW, VT::MultiViewMedium::TEXTURE_ARRAY, VT::MultiViewChange::LAYER, VT::MultiViewPassThrough::DISABLED, RP501)

// @TODO: Make as many depth techniques as there are view techniques. Experiment thoroughly.

using DT = DepthTechnique;

ADD_DEPTH_TECHNIQUE(DT::DepthLayerSorting::MULTIPASS, DT::DepthLayerMedium::SEPARATE_TEXTURES, DT::DepthLayerChange::FRAMEBUFFER, RP701)
