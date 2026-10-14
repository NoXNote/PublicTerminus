#pragma once

#include "rengb_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace rengb
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		float scale;
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
	};

	class RengbGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, RengbGameObject>;


		static RengbGameObject createGameObject()
		{
			static id_t currentId = 0;
			return RengbGameObject{currentId++};
		}

		RengbGameObject(const RengbGameObject &) = delete;
		RengbGameObject &operator=(const RengbGameObject &) = delete;
		RengbGameObject(RengbGameObject &&) = default;
		RengbGameObject &operator=(RengbGameObject &&) = default;

		id_t getId() { return id; }

		glm::vec3 color{};
		TransformComponent transform{};

		// Optional pointer components
		std::shared_ptr<RengbModel> model{};


	private:
		RengbGameObject(id_t objId) : id{objId} {}

		id_t id;
	};

} 