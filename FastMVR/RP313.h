#pragma once

#include "RP312a.h"

class RP313 : public RP312a
{
protected:

	virtual void start() override;

public:

	RP313(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices, GeometryProviderBase& geometry);

	virtual void begin() override;

};