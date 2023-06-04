#pragma once
#include "Volt/Scene/Scene.h"

#include <Wire/Entity.h>
#include <Wire/WireGUID.h>

#include <unordered_map>

namespace Volt
{
	class PhysicsActor;
	class MonoScriptFieldCache;

	class Entity
	{
	public:
		Entity();
		Entity(Wire::EntityId id, Scene* scene);
		Entity(const Entity& entity);

		~Entity();

		template<typename T>
		T& GetComponent();

		template<typename T>
		T& GetComponent() const;

		const std::string GetTag();
		void SetTag(const std::string& tag);

		const gem::mat4 GetTransform() const;
		const gem::mat4 GetLocalTransform() const;

		const gem::vec3 GetForward() const;
		const gem::vec3 GetRight() const;
		const gem::vec3 GetUp() const;

		const gem::vec3 GetLocalForward() const;
		const gem::vec3 GetLocalRight() const;
		const gem::vec3 GetLocalUp() const;

		const gem::vec3 GetLocalPosition() const;
		const gem::quat GetLocalRotation() const;
		const gem::vec3 GetLocalScale() const;

		const gem::vec3 GetPosition() const;
		const gem::quat GetRotation() const;
		const gem::vec3 GetScale() const;

		const bool IsVisible() const;
		void SetVisible(bool state);
		void SetLocked(bool state);

		const uint32_t GetLayerId() const;

		const std::vector<Volt::Entity> GetChilden() const;
		const Volt::Entity GetParent();
		
		void ResetParent();
		void ResetChildren();

		void RemoveChild(Volt::Entity entity);

		const Ref<PhysicsActor> GetPhysicsActor() const;

		void UpdatePhysicsTranslation(bool updateThis);
		void UpdatePhysicsRotation(bool updateThis);

		void SetPosition(const gem::vec3& position, bool updatePhysics = true);
		void SetRotation(const gem::quat& rotation, bool updatePhysics = true);
		void SetScale(const gem::vec3& scale);

		void SetLocalPosition(const gem::vec3& position, bool updatePhysics = true);
		void SetLocalRotation(const gem::quat& rotation, bool updatePhysics = true);
		void SetLocalScale(const gem::vec3& scale);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		bool HasComponent() const;

		template<typename T>
		void RemoveComponent();

		inline const Wire::EntityId GetId() const { return myId; }
		inline bool IsNull() const
		{
			if (myId == Wire::NullID)
			{
				return true;
			}

			if (!myScene || !myScene->GetRegistry().Exists(myId))
			{
				return true;
			}

			return false;
		}
		inline Scene* GetScene() const { return myScene; };

		Entity& operator=(const Entity& entity);

		inline bool operator==(const Entity& entity) const { return myId == entity.myId; }
		inline bool operator!() const { return IsNull(); }
		inline explicit operator bool() const { return !IsNull(); }

		// Copies from one entity to another
		static void Copy(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);

		// Duplicated an entire entity tree
		static Wire::EntityId Duplicate(Wire::Registry& aRegistry, MonoScriptFieldCache& scriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity = Wire::NullID, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);
		static Wire::EntityId Duplicate(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity = Wire::NullID, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);

		static Volt::Entity Null() { return { 0, nullptr }; }

	private:
		static void CopyPhysicsComponents(const WireGUID& guid, Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity);
		static Wire::EntityId DuplicateInternal(Wire::Registry& aRegistry, MonoScriptFieldCache& scriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aParent, Wire::EntityId aTargetEntity = Wire::NullID, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);
		static Wire::EntityId DuplicateInternal(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, MonoScriptFieldCache& scrScriptFieldCache, MonoScriptFieldCache& targetScriptFieldCache, Wire::EntityId aSrcEntity, Wire::EntityId aParent, Wire::EntityId aTargetEntity = Wire::NullID, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);

		Scene* myScene = nullptr;
		Wire::EntityId myId = Wire::NullID;
	};

	template<typename T>
	inline T& Entity::GetComponent()
	{
		return myScene->myRegistry.GetComponent<T>(myId);
	}

	template<typename T>
	inline T& Entity::GetComponent() const
	{
		return myScene->myRegistry.GetComponent<T>(myId);
	}

	template<typename T, typename... Args>
	inline T& Entity::AddComponent(Args&&... args)
	{
		return myScene->myRegistry.AddComponent<T>(myId, std::forward<Args>(args)...);
	}

	template<typename T>
	inline bool Entity::HasComponent() const
	{
		return myScene->myRegistry.HasComponent<T>(myId);
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		myScene->myRegistry.RemoveComponent<T>(myId);
	}
}
