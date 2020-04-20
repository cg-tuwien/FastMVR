#include "AdditionalShaderCode.h"

AdditionalShaderCode add_code()
{
	return AdditionalShaderCode{};
}

AdditionalShaderCode& AdditionalShaderCode::to_all(std::string_view code, bool encloseNewLines)
{
	to_vert(code, encloseNewLines);
	to_geom(code, encloseNewLines);
	to_frag(code, encloseNewLines);
	return *this;
}

AdditionalShaderCode& AdditionalShaderCode::to_vert(std::string_view code, bool encloseNewLines)
{
	if (encloseNewLines) { m_additionToVertexShader += "\n"; }
	m_additionToVertexShader += code;
	if (encloseNewLines) { m_additionToVertexShader += "\n"; }
	return *this;
}

AdditionalShaderCode& AdditionalShaderCode::to_geom(std::string_view code, bool encloseNewLines)
{
	if (encloseNewLines) { m_additionToGeometryShader += "\n"; }
	m_additionToGeometryShader += code;
	if (encloseNewLines) { m_additionToGeometryShader += "\n"; }
	return *this;
}

AdditionalShaderCode& AdditionalShaderCode::to_frag(std::string_view code, bool encloseNewLines)
{
	if (encloseNewLines) { m_additionToFragementShader += "\n"; }
	m_additionToFragementShader += code;
	if (encloseNewLines) { m_additionToFragementShader += "\n"; }
	return *this;
}

