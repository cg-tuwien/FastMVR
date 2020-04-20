#include "SceneLoader.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <stack>
#include <string_view>
#include "FileHelper.h"
#include "glm/gtc/matrix_access.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

SceneLoader::SceneLoader(const char* file) : filename(file)
{
	std::filesystem::path p(file);
	directory = p.parent_path();
}

void SceneLoader::parse(const std::string c, Node& startnode)
{
	std::stack<Node*> ns;
	ns.push(&startnode);

	std::regex relevant("\\}|\\s*(\\w+)\\s*=\\s*(\\S+)");
	for (auto it = std::sregex_iterator(c.begin(), c.end(), relevant), end = std::sregex_iterator(); ns.size() && it != end; it++)
	{
		std::sub_match first = it->operator[](1), second = it->operator[](2);
		if (first.matched)
		{
			if (second != "{") //Actual assignment
			{
				ns.top()->assignments[first] = second;
			}
			else
			{
				ns.push(&ns.top()->nodes[first]);
			}
		}
		else
		{
			ns.pop();
		}
	}
}

void SceneLoader::buildModel(
	const tinyobj::attrib_t& attrib,
	const std::vector<tinyobj::shape_t>& shapes, 
	std::map<tinyobj::index_t, uint32_t, IDXComp>& v2i,
	std::vector<SceneVertex>& model_vertices,
	std::vector<uint32_t>& model_indices,
	uint32_t& m_v_off,
	uint32_t& m_i_off
	)
{
	m_v_off = 0;
	m_i_off = 0;
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
		uint32_t s_i_off = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];
			if (fv != 3)
			{
				throw std::runtime_error("Alarm! Found non-triangle faces!");
			}
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[s_i_off + v];
				if (v2i.find(idx) == v2i.end())
				{
					tinyobj::real_t vx, vy, vz;
					vx = attrib.vertices[3 * idx.vertex_index + 0];
					vy = attrib.vertices[3 * idx.vertex_index + 1];
					vz = attrib.vertices[3 * idx.vertex_index + 2];

					tinyobj::real_t nx, ny, nz;
					if (idx.normal_index != -1)
					{
						nx = attrib.normals[3 * idx.normal_index + 0];
						ny = attrib.normals[3 * idx.normal_index + 1];
						nz = attrib.normals[3 * idx.normal_index + 2];
					}
					else
					{
						nx = ny = nz = 0;
					}

					SceneVertex v = { {vx, vy, vz}, {nx, ny, nz} };
					model_vertices[m_v_off] = v;
					v2i[idx] = m_v_off;
					m_v_off++;
				}
				model_indices[m_i_off] = v2i[idx];
				m_i_off++;
			}
			s_i_off += fv;
		}
	}
}

void SceneLoader::readMatrices(const Node& node, std::vector<glm::mat4>& matrices)
{
	if (node.assignments.find("model_matrices") != node.assignments.end())
	{
		std::string m = node.assignments.at("model_matrices");
		m.erase(remove(m.begin(), m.end(), '\"'), m.end());
		std::filesystem::path matrixfile = m;
		matrixfile = directory / matrixfile;

		std::vector<uint8_t> bytes;
		FileHelper::readFullFileBinary(matrixfile.string().c_str(), bytes);

		if (matrices.size() != 1)
		{
			std::cout << "Multiple!";
		}
		uint32_t N = *reinterpret_cast<uint32_t*>(bytes.data());
		matrices.resize(N);
		std::memcpy(matrices.data(), bytes.data() + sizeof(uint32_t), sizeof(glm::mat4) * N);
	}
}

void SceneLoader::readScene(uint32_t& read_materials, uint32_t& read_objects, uint32_t& read_triangles)
{
	if (!std::filesystem::is_regular_file(filename))
	{
		throw std::runtime_error("Could not find named scene file!");
	}

	if (restore())
	{
		return;
	}

	std::string c;
	FileHelper::readFullFile(filename.string().c_str(), c);

	Node cfg = {};
	parse(c, cfg);

	const Node& models = cfg.nodes["scene"].nodes["models"];
	for (const auto&[k, n] : models.nodes)
	{
		std::string s = n.assignments.at("mesh");
		s.erase(remove(s.begin(), s.end(), '\"'), s.end());
		std::filesystem::path inputfile = s;
		inputfile = directory / inputfile;

		std::string warn;
		std::string err;
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.string().c_str());

		if (!err.empty()) {
			std::cerr << err << std::endl;
			throw std::runtime_error("Error loading obj");
		}

		if (!ret) {
			throw std::runtime_error("Error in tinyobj");
		}

		std::map<tinyobj::index_t, uint32_t, IDXComp> v2i;
		uint32_t num_shape_indices = 0;
		for (int s = 0; s < shapes.size(); s++)
		{
			num_shape_indices += (uint32_t)shapes[s].mesh.indices.size();
		}

		std::vector<SceneVertex> model_vertices(num_shape_indices);
		std::vector<uint32_t> model_indices(num_shape_indices);

		uint32_t m_v_off, m_i_off;
		buildModel(attrib, shapes, v2i, model_vertices, model_indices, m_v_off, m_i_off);

		std::vector<glm::mat4> matrices(1, glm::mat4(1.0f));
		readMatrices(n, matrices);

		uint32_t old_v_size = (uint32_t)vertices.size();
		uint32_t old_i_size = (uint32_t)indices.size();

		vertices.resize(old_v_size + m_v_off);
		for (uint32_t v = 0; v < m_v_off; v++)
		{
			vertices[old_v_size + v] = { model_vertices[v].position, model_vertices[v].normal };
		}

		indices.resize(old_i_size + m_i_off);
		for (uint32_t i = 0; i < m_i_off; i++)
		{
			indices[old_i_size + i] = old_v_size + model_indices[i];
		}

		//Baking matrix into vertices
		glm::mat4 trans = matrices[0];
		glm::mat4 rot = trans;
		rot = glm::row(rot, 3, glm::vec4(0, 0, 0, 1));
		rot = glm::column(rot, 3, glm::vec4(0, 0, 0, 1));
		rot = glm::column(rot, 0, glm::normalize(glm::column(rot, 0)));
		rot = glm::column(rot, 1, glm::normalize(glm::column(rot, 1)));
		rot = glm::column(rot, 2, glm::normalize(glm::column(rot, 2)));

		baked_vertices.resize(old_v_size + m_v_off);
		for (uint32_t v = 0; v < m_v_off; v++)
		{
			glm::vec4 position = trans * glm::vec4(model_vertices[v].position, 1);
			glm::vec4 normal = rot * glm::vec4(model_vertices[v].normal, 1);
			baked_vertices[old_v_size + v] = { {position.x, position.y, position.z},{normal.x, normal.y, normal.z} };
		}

		SceneObject object;
		object.fromTo = { old_i_size, old_i_size + m_i_off };
		object.matrices = matrices;
		objects.push_back(object);
	}

	dump();
}

void SceneLoader::dump()
{
	uint32_t total_v_size = (uint32_t)vertices.size();
	uint32_t total_i_size = (uint32_t)indices.size();
	uint32_t total_o_size = (uint32_t)objects.size();

	std::filesystem::path dumpfile(filename.string() + ".dump");
	std::ofstream df(dumpfile, std::ios_base::out | std::ios_base::binary);
	df.write((char*)&total_v_size, sizeof(uint32_t));
	df.write((char*)vertices.data(), total_v_size * sizeof(SceneVertex));
	df.write((char*)baked_vertices.data(), total_v_size * sizeof(SceneVertex));
	df.write((char*)&total_i_size, sizeof(uint32_t));
	df.write((char*)indices.data(), total_i_size * sizeof(uint32_t));
	df.write((char*)&total_o_size, sizeof(uint32_t));
	
	for (uint32_t i = 0; i < total_o_size; i++)
	{
		uint32_t l_size = (uint32_t)objects[i].matrices.size();
		df.write((char*)&objects[i].fromTo, sizeof(glm::uvec2));
		df.write((char*)&l_size, sizeof(uint32_t));
		df.write((char*)objects[i].matrices.data(), sizeof(glm::mat4) * l_size);
	}
}

bool SceneLoader::restore()
{
	std::filesystem::path dumpfile(filename.string() + ".dump");
	std::ifstream df(dumpfile, std::ios_base::in | std::ios_base::binary);

	if (df.good())
	{
		uint32_t total_v_size, total_i_size, total_o_size;
		df.read((char*)&total_v_size, sizeof(uint32_t));
		vertices.resize(total_v_size);
		baked_vertices.resize(total_v_size);
		df.read((char*)vertices.data(), total_v_size * sizeof(SceneVertex));
		df.read((char*)baked_vertices.data(), total_v_size * sizeof(SceneVertex));
		df.read((char*)&total_i_size, sizeof(uint32_t));
		indices.resize(total_i_size);
		df.read((char*)indices.data(), total_i_size * sizeof(uint32_t));
		df.read((char*)&total_o_size, sizeof(uint32_t));

		objects.resize(total_o_size);
		for (uint32_t i = 0; i < total_o_size; i++)
		{
			uint32_t l_size;
			df.read((char*)&objects[i].fromTo, sizeof(glm::uvec2));
			df.read((char*)&l_size, sizeof(uint32_t));
			objects[i].matrices.resize(l_size);
			df.read((char*)objects[i].matrices.data(), sizeof(glm::mat4) * l_size);
		}
		return true;
	}

	return false;
}