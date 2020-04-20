#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <filesystem>
#include "Scene.h"
#include <tiny_obj_loader.h>
#include <map>

class SceneLoader
{
public:

	SceneLoader(const char* file);

	void readScene(uint32_t& read_materials, uint32_t& read_objects, uint32_t& read_triangles);

	const std::vector<SceneVertex>& getSceneGeometryBaked()
	{
		return baked_vertices;
	}

	const std::vector<SceneVertex>& getSceneGeometry()
	{
		return vertices;
	}

	const std::vector<uint32_t>& getSceneIndices()
	{
		return indices;
	}

	const std::vector<SceneObject>& getObjects()
	{
		return objects;
	}

private:

	struct IDXComp
	{
		bool operator()(const tinyobj::index_t& left, const tinyobj::index_t& right) const
		{
			if (left.vertex_index == right.vertex_index)
			{
				if (left.normal_index == right.normal_index)
				{
					return left.texcoord_index < right.texcoord_index;
				}
				return left.normal_index < right.normal_index;
			}
			return left.vertex_index < right.vertex_index;
		}
	};

	struct Node
	{
		std::map<std::string, Node> nodes;
		std::map<std::string, std::string> assignments;
	};

	void dump();

	bool restore();

	void parse(const std::string c, Node& startnode);

	void buildModel
	(
		const tinyobj::attrib_t& attrib,
		const std::vector<tinyobj::shape_t>& shapes,
		std::map<tinyobj::index_t, uint32_t, IDXComp>& v2i,
		std::vector<SceneVertex>& model_vertices,
		std::vector<uint32_t>& model_indices,
		uint32_t& m_v_off, uint32_t& m_i_off
	);

	void readMatrices(const Node& node, std::vector<glm::mat4>& matrices);

	std::filesystem::path filename;
	std::filesystem::path directory;
	std::vector<SceneVertex> vertices;
	std::vector<SceneVertex> baked_vertices;
	std::vector<uint32_t> indices;
	std::vector<SceneObject> objects;
};