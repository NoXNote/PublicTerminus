#pragma once

#include "rengb_component_manager.hpp"
#include "rengb_entity_manager.hpp"
#include "rengb_system_manager.hpp"
#include "rengb_utils.hpp"
#include <memory>


namespace rengb {
	class Coordinator
	{
	public:
		void Init()
		{
			// Create pointers to each manager
			mComponentManager = std::make_unique<ComponentManager>();
			mEntityManager = std::make_unique<EntityManager>();
			mSystemManager = std::make_unique<SystemManager>();
		}

		// Entity methods
		Entity CreateEntity()
		{
			return mEntityManager->CreateEntity();
		}

		void DestroyEntity(Entity entity)
		{
			mEntityManager->DestroyEntity(entity);
			// std::cout<<"destroyed in entity manager"<<std::endl;
			mComponentManager->EntityDestroyed(entity);
			// std::cout<<"destroyed in comonent manager"<<std::endl;
			mSystemManager->EntityDestroyed(entity);
			// std::cout<<"destroyed in system manager"<<std::endl;
		}

		// Component methods
		template <typename T>
		void RegisterComponent()
		{
			mComponentManager->RegisterComponent<T>();
		}

		template <typename T>
		void AddComponent(Entity entity, T component)
		{
			//std::cout << "20" << std::endl;
			mComponentManager->AddComponent<T>(entity, component);
			//std::cout << "30" << std::endl;
			auto signature = mEntityManager->GetSignature(entity);
			//std::cout << "40" << std::endl;
			signature.set(mComponentManager->GetComponentType<T>(), true);
			//std::cout << "50" << std::endl;
			mEntityManager->SetSignature(entity, signature);
			//std::cout << "60" << std::endl;

			mSystemManager->EntitySignatureChanged(entity, signature);
			//std::cout << "70" << std::endl;
		}

		template <typename T>
		void RemoveComponent(Entity entity)
		{
			mComponentManager->RemoveComponent<T>(entity);

			auto signature = mEntityManager->GetSignature(entity);
			signature.set(mComponentManager->GetComponentType<T>(), false);
			mEntityManager->SetSignature(entity, signature);

			mSystemManager->EntitySignatureChanged(entity, signature);
		}

		template <typename T>
		T &GetComponent(Entity entity)
		{
			return mComponentManager->GetComponent<T>(entity);
		}

		template <typename T>
		ComponentType GetComponentType()
		{
			return mComponentManager->GetComponentType<T>();
		}

		Signature GetSignature(Entity entity)
		{
			return mEntityManager->GetSignature(entity);
		}

		template <typename T>
		bool HasComponent(Entity entity)
		{
			Signature signature;
			signature.set(mComponentManager->GetComponentType<T>(), true);
			return ((GetSignature(entity) & signature )== signature);
		}

		// System methods
		template <typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			return mSystemManager->RegisterSystem<T>();
		}

		template <typename T>
		std::shared_ptr<T> RegisterRenderSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		{
			return mSystemManager->RegisterRenderSystem<T>(device, renderPass, globalSetLayout);
		}

		template <typename T>
		void SetSystemSignature(Signature signature)
		{
			mSystemManager->SetSignature<T>(signature);
		}

		// Clears all components, therefore destroys all Vulkan objects
		template <typename T>
		void ClearArray()
		{
			mComponentManager->ClearArray<T>();
		}

	private:
		std::unique_ptr<ComponentManager> mComponentManager;
		std::unique_ptr<EntityManager> mEntityManager;
		std::unique_ptr<SystemManager> mSystemManager;
	};
}

