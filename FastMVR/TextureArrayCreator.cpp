#include "PipelineBehaviors.h"	


{
	resources = r;

	textures = { std::vector<int>(1) };
	renderbuffers = { std::vector<int>(1) };
	framebuffers = { std::vector<int>(1) };

	RenderResources::ArrayTextureConfig c_config;
	c_config.width = width;
	c_config.height = height;
	c_config.num_layers = view_matrices.size();
	c_config.format = RenderResources::TextureFormat::R32UI;

	RenderResources::ArrayTextureConfig d_config(width, height, view_matrices.size(), RenderResources::TextureFormat::DEPTH);
	RenderResources::FramebufferConfig f_config;

	textures[0][0] = resources->buildArrayTexture(c_config);
	renderbuffers[0][0] = resources->buildArrayTexture(d_config);
	framebuffers[0][0] = resources->buildFramebuffer(f_config);

	resources->combineLayered(framebuffers[0][0], textures[0][0], renderbuffers[0][0]);
	resources->enableTargets(framebuffers[0][0], { 0 });

	if (!resources->checkFramebuffer(framebuffers[0][0]))
	{
		throw std::runtime_error("Problem making framebuffer!");
	}
	}