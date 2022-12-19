#pragma once
#include "Volt/Scene/Scene.h"

#include <Wire/Entity.h>
#include <Wire/WireGUID.h>

#include "Volt/Scripting/ScriptEngine.h"
#include "Volt/Scripting/ScriptRegistry.h"

#include <unordered_map>

namespace Volt
{
	class PhysicsActor;
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

		template<typename T>
		T* GetScript();

		bool HasScript(const std::string& scriptName);
		void AddScript(const std::string& scriptName);
		void RemoveScript(const std::string& scriptName);
		void RemoveScript(WireGUID scriptGUID);

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

		const std::vector<Volt::Entity> GetChilden() const;
		const Volt::Entity GetParent() const;

		const Ref<PhysicsActor> GetPhysicsActor() const;

		void SetPosition(const gem::vec3& position, bool updatePhysics = true);

		void SetLocalPosition(const gem::vec3& position, bool updatePhysics = true);
		void SetLocalRotation(const gem::quat& rotation);
		void SetLocalScale(const gem::vec3& scale);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		bool HasComponent();

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
		static void Copy(Wire::Registry& aSrcRegistry, Wire::Registry& aTargetRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aTargetEntity, std::vector<WireGUID> aExcludedComponents = std::vector<WireGUID>(), bool shouldReset = false);

		// Duplicated an entire entity tree
		static Wire::EntityId Duplicate(Wire::Registry& aRegistry, Wire::EntityId aSrcEntity);

		static Volt::Entity GetNullEntity() { return Volt::Entity(0, nullptr); }

	private:
		static Wire::EntityId DuplicateInternal(Wire::Registry& aRegistry, Wire::EntityId aSrcEntity, Wire::EntityId aParent);

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

	template<typename T>
	inline T* Entity::GetScript()
	{
		const auto guid = T::GetStaticGUID();
		if (!ScriptEngine::IsScriptRegistered(guid, myId))
		{
			return nullptr;
		}

		ScriptBase* script = ScriptEngine::GetScript(myId, guid).get();
		return reinterpret_cast<T*>(script);
	}

	template<typename T, typename... Args>
	inline T& Entity::AddComponent(Args&&... args)
	{
		return myScene->myRegistry.AddComponent<T>(myId, std::forward<Args>(args)...);
	}

	template<typename T>
	inline bool Entity::HasComponent()
	{
		return myScene->myRegistry.HasComponent<T>(myId);
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		myScene->myRegistry.RemoveComponent<T>(myId);
	}
}