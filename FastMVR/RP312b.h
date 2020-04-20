#pragma once

#include "RP202.h"

class RP313 : public RP202
{
protected:

public:

	RP313(
		uint32_t width,
		uint32_t height,
		std::vector<glm::mat4> view_matrices);

	virtual void begin() override;

	virtual void produce() override;
};