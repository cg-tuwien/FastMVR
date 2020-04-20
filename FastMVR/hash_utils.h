#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

/*	Combines multiple hash values.
 *  Inspiration and implementation largely taken from: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/54728293#54728293
 *  TODO: Should a larger magic constant be used since we're only supporting x64 and hence, size_t will always be 64bit?!
 */
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
	seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
}

namespace std
{
	template<> struct hash<vector<glm::mat4>>
	{
		std::size_t operator()(vector<glm::mat4> const& v) const noexcept
		{
			std::size_t h = 0;
			for (auto& m : v) {
				hash_combine(h, m);
			}
			return h;
		}
	};
}




