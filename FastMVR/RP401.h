#pragma once

#include "RP402.h"
#include <cstdint>
#include <vector>
#include <glm/matrix.hpp>

class RP401 : public RP402
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

	virtual void doViewPosition(uint32_t view) override;

	virtual void start() override;

public:

	RP401(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void begin() override;
};
