#pragma once
#include <string>


struct AdditionalShaderCode
{
	AdditionalShaderCode& to_all(std::string_view code, bool encloseNewLines = true);
	AdditionalShaderCode& to_vert(std::string_view code, bool encloseNewLines = true);
	AdditionalShaderCode& to_geom(std::string_view code, bool encloseNewLines = true);
	AdditionalShaderCode& to_frag(std::string_view code, bool encloseNewLines = true);

	std::string m_additionToVertexShader;
	std::string m_additionToGeometryShader;
	std::string m_additionToFragementShader;
};

AdditionalShaderCode add_code();
