#pragma once
#include "RP202.h"

class RP203 : public RP202
{
protected:

	static const char* vs_GL;
	static const char* gs_GL;

	virtual void start() override;
	virtual void doViewPosition(uint32_t view) override;

	virtual std::array<std::string, 3> getShaders() override;

public:

	RP203(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);
};