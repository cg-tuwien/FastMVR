#pragma once

#include "RP001.h"
#include <cstdint>
#include <vector>
#include <glm/matrix.hpp>

class RP402 : public RP001
{
protected:

	virtual std::array<std::string, 3> getShaders() override;

	virtual void start() override;

	virtual void doViewPosition(uint32_t view) override;

public:

	RP402(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);


	virtual void begin() override;

	virtual void init(RenderResources* r) override;

	virtual void dumpImages(const char* directory) override;
};
