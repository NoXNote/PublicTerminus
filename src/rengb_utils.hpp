#pragma once

#include <functional>
#include <bitset>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#define HALF_PI 1.57079632679489661923132169163975144
#define PI 3.14159265358979323846264338327950288
#define THREE_OVER_TWO_PI 4.71238898038468985769396507491925432
#define TWO_PI 6.28318530717958647692528676655900576


namespace rengb 
{
	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
	{
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};



	enum partTypes {weapon, body, head, leg, arm};
	enum enemyTypes {light, medium, heavy, miniboss};
	

	// ECS
	using Entity = std::uint32_t;
	Entity const MAX_ENTITIES = 1024;
	using ComponentType = std::uint8_t;
	ComponentType const MAX_COMPONENTS = 32;
	using Signature = std::bitset<MAX_COMPONENTS>;
}

