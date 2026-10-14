#pragma once

#include "rengb_system.hpp"
#include "rengb_device.hpp"
#include "rengb_utils.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace rengb {
	class SystemManager
	{
	public:
		template <typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char *typeName = typeid(T).name();

			assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

			// Create a pointer to the system and return it so it can be used externally
			auto system = std::make_shared<T>();
			mSystems.insert({typeName, system});
			return system;
		}

		template <typename T>
		std::shared_ptr<T> RegisterRenderSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		{
			const char *typeName = typeid(T).name();

			assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

			// Create a pointer to the system and return it so it can be used externally
			auto system = std::make_shared<T>(device, renderPass, globalSetLayout);
			mSystems.insert({typeName, system});
			return system;
		}

		template <typename T>
		void SetSignature(Signature signature)
		{
			const char *typeName = typeid(T).name();

			assert(mSystems.find(typeName) != mSystems.end() && "System used before registered.");

			// Set the signature for this system
			mSignatures.insert({typeName, signature});
		}

		void EntityDestroyed(Entity entity)
		{
			// Erase a destroyed entity from all system lists
			// mEntities is a set so no check needed
			for (auto const &pair : mSystems)
			{
				auto const &system = pair.second;

				system.lock()->mEntities.erase(entity);
			}
		}

		void EntitySignatureChanged(Entity entity, Signature entitySignature)
		{
			// Notify each system that an entity's signature changed
			for (auto const &pair : mSystems)
			{
				// std::cout << "a" << std::endl;
				auto const &type = pair.first;
				// std::cout << "b" << std::endl;
				auto const &system = pair.second.lock();
				// std::cout << "c" << std::endl;
				auto const &systemSignature = mSignatures[type];
				// std::cout << "d" << std::endl;

				// Entity signature matches system signature - insert into set
				if ((entitySignature & systemSignature) == systemSignature)
				{
					//std::cout<<std::endl<<"entity sig changed"<<std::endl;
					// std::cout << "e" << std::endl;
					system->mEntities.insert(entity);
				}
				// Entity signature does not match system signature - erase from set
				else
				{
					// std::cout << "f" << std::endl;
					system->mEntities.erase(entity);
				}
				// std::cout << "g" << std::endl;
			}
		}



	private:
		// Map from system type string pointer to a signature
		std::unordered_map<const char *, Signature> mSignatures{};

		// Map from system type string pointer to a system pointer
		std::unordered_map<const char *, std::weak_ptr<System>> mSystems{};
	};
}

