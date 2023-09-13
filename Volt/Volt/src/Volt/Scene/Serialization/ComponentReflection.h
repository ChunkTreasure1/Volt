#pragma once

#include "Volt/Scene/Serialization/ComponentGUID.h"
#include "Volt/Utility/MemoryUtility.h"

namespace Volt
{
	class IDefaultValueType
	{
	public:
		IDefaultValueType() = default;
		virtual ~IDefaultValueType() = default;

		virtual const void* Get() const = 0;
	};

	template<typename T>
	class DefaultValueType : public IDefaultValueType
	{
	public:
		inline DefaultValueType(const T& value)
			: m_defaultValue(value)
		{}
		~DefaultValueType() override = default;

		inline const void* Get() const override { return &m_defaultValue; };

	private:
		T m_defaultValue;
	};

	struct ComponentMember
	{
		ptrdiff_t offset;
		uint32_t id = ~0u;
		std::string_view name;
		std::string_view label;
		std::string_view description;
	
		Scope<IDefaultValueType> defaultValue;
	};

	class IComponentDesc
	{
	public:
		virtual ~IComponentDesc() = default;

		[[nodiscard]] virtual const VoltGUID& GetGUID() const = 0;
		[[nodiscard]] virtual const std::string_view GetLabel() const = 0;
		[[nodiscard]] virtual const std::string_view GetDescription() const = 0;
		[[nodiscard]] virtual const std::vector<ComponentMember>& GetMembers() const = 0;
	};

	template<typename T>
	class ComponentDesc : public IComponentDesc
	{
	public:
		void SetLabel(std::string_view name);
		void SetDescription(std::string_view description);
		void SetGUID(const VoltGUID& guid);

		[[nodiscard]] inline const VoltGUID& GetGUID() const override { return m_guid; }
		[[nodiscard]] inline const std::string_view GetLabel() const override { return m_componentLabel; }
		[[nodiscard]] inline const std::string_view GetDescription() const override { return m_componentDescription; }
		[[nodiscard]] inline const std::vector<ComponentMember>& GetMembers() const override { return m_members; }

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, uint32_t id, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue)
		{
			static ComponentMember* nullMember = nullptr;

			const ptrdiff_t offset = Utility::GetMemberOffset(memberPtr);
			const bool hasMemberWithOffset = FindMemberByOffset(offset) != nullptr;
			VT_CORE_ASSERT(!hasMemberWithOffset, "Member with offset has already been registered")
			if (hasMemberWithOffset)
			{
				return *nullMember;
			}

			const bool hasMemberWithId = FindMemberById(id) != nullptr;
			VT_CORE_ASSERT(!hasMemberWithId, "Member with id has already been registered!");
			if (hasMemberWithId)
			{
				return *nullMember;
			}

			const bool hasMemberWithName = FindMemberByName(name) != nullptr;
			VT_CORE_ASSERT(!hasMemberWithName, "Member with name has already been registered!");
			if (hasMemberWithName)
			{
				return *nullMember;
			}

			m_members.emplace_back(offset, id, name, label, description, CreateScope<DefaultValueType<DefaultValueT>>(defaultValue));
			
			return m_members.back();
		}

	private:
		ComponentMember* FindMemberByOffset(const ptrdiff_t offset);
		ComponentMember* FindMemberById(const uint32_t id);
		ComponentMember* FindMemberByName(std::string_view name);

		std::vector<ComponentMember> m_members;

		VoltGUID m_guid = VoltGUID::Null();
		std::string m_componentLabel;
		std::string m_componentDescription;

	};

	template<typename T>
	inline void ComponentDesc<T>::SetLabel(std::string_view name)
	{
		m_componentLabel = name;
	}

	template<typename T>
	inline void ComponentDesc<T>::SetDescription(std::string_view description)
	{
		m_componentDescription = description;
	}
	
	template<typename T>
	inline void ComponentDesc<T>::SetGUID(const VoltGUID& guid)
	{
		m_guid = guid;
	}
	
	template<typename T>
	inline ComponentMember* ComponentDesc<T>::FindMemberByOffset(const ptrdiff_t offset)
	{
		auto it = std::find_if(m_members.begin(), m_members.end(), [offset](const auto& member) 
		{
			return member.offset == offset;
		});

		if (it == m_members.end())
		{
			return nullptr;
		}

		return &(*it);
	}
	
	template<typename T>
	inline ComponentMember* ComponentDesc<T>::FindMemberById(const uint32_t id)
	{
		auto it = std::find_if(m_members.begin(), m_members.end(), [id](const auto& member)
		{
			return member.id == id;
		});

		if (it == m_members.end())
		{
			return nullptr;
		}

		return &(*it);
	}
	
	template<typename T>
	inline ComponentMember* ComponentDesc<T>::FindMemberByName(std::string_view name)
	{
		auto it = std::find_if(m_members.begin(), m_members.end(), [name](const auto& member)
		{
			return member.name == name;
		});

		if (it == m_members.end())
		{
			return nullptr;
		}

		return &(*it);
	}
}
