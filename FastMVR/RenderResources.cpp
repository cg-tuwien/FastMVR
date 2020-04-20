#include "RenderResources.h"
#include "Scene.h"
#include <string>

RenderResources::RenderResources(const SceneObject* objects, uint32_t num_objects)
	:
	current_object(0), current_matrix(0)
{
}

const SceneObject* RenderResources::getObject()
{
	return scene_objects[current_object];
}

const glm::mat4& RenderResources::getMatrix()
{
	return scene_objects[current_object]->matrices[current_matrix];
}

bool RenderResources::advanceObjectMatrix()
{
	if (current_matrix < scene_objects[current_object]->matrices.size() - 1)
	{
		current_matrix++;
	}
	else
	{
		current_matrix = 0;
		if (current_object < scene_objects.size() - 1)
		{
			current_object++;
		}
		else
		{
			current_object = 0;
			return false;
		}
	}
	return true;
}

bool RenderResources::advanceObject()
{
	current_matrix = 0;
	if (current_object < scene_objects.size() - 1)
	{
		current_object++;
		return true;
	}
	else
	{
		current_object = 0;
		return false;
	}
}