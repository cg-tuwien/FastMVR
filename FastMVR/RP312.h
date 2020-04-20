#pragma once

#include "RP202.h"

class RP312 : public RP202
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;

	virtual std::array<std::string, 3> getShaders() override;

	virtual void start() override;

public:

	RP312(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);
};