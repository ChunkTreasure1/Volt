#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include "Volt/Scene/Serialization/VoltGUID.h"
#include "Volt/Scene/Serialization/TypeAliases.h"
#include "Volt/Utility/MemoryUtility.h"

#include "Volt/Asset/Asset.h"

#include <typeindex>

namespace Volt
{
	template<typename T>
	concept Enum = std::is_enum<T>::value;

	template<typename T, class ENABLE = void> class TypeDesc;
	template<typename T> constexpr bool IsReflectedType();
	template<typename T> const TypeDesc<T>* GetTypeDesc();

	template<typename T> inline constexpr auto ReflectType(TypeDesc<T>& desc) -> decltype(T::ReflectType(std::declval<TypeDesc<T>&>()), void())
	{
		T::ReflectType(desc);
	}

	enum class ValueType
	{
		Default,
		Component,
		Enum
	};

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
		{
		}
		~DefaultValueType() override = default;

		inline const void* Get() const override { return &m_defaultValue; };

	private:
		T m_defaultValue;
	};

	class ICommonTypeDesc
	{
	public:
		virtual ~ICommonTypeDesc() = default;

		[[nodiscard]] virtual const VoltGUID& GetGUID() const = 0;
		[[nodiscard]] virtual const std::string_view GetLabel() const = 0;
		[[nodiscard]] virtual const std::string_view GetDescription() const = 0;
		[[nodiscard]] virtual const ValueType GetValueType() const = 0;
	};

	template<ValueType VALUE_TYPE>
	class CommonTypeDesc : public ICommonTypeDesc
	{
	public:
		~CommonTypeDesc() override = default;

		[[nodiscard]] const ValueType GetValueType() const override { return m_valueType; }

	protected:
		const ValueType m_valueType = VALUE_TYPE;
	};

	struct ComponentMember
	{
		ptrdiff_t offset;
		std::string_view name;
		std::string_view label;
		std::string_view description;

		AssetType assetType = AssetType::None;

		const ICommonTypeDesc* typeDesc = nullptr;
		std::type_index typeIndex = typeid(void);
		Scope<IDefaultValueType> defaultValue;
	};

	struct EnumConstant
	{
		int32_t value;
		std::string_view name;
		std::string_view label;
	};

	class IComponentTypeDesc : public CommonTypeDesc<ValueType::Component>
	{
	public:
		~IComponentTypeDesc() override = default;

		[[nodiscard]] virtual const std::vector<ComponentMember>& GetMembers() const = 0;
		[[nodiscard]] virtual const bool IsHidden() const = 0;
	};

	class IEnumTypeDesc : public CommonTypeDesc<ValueType::Enum>
	{
	public:
		~IEnumTypeDesc() override = default;

		[[nodiscard]] virtual const std::vector<EnumConstant>& GetConstants() const = 0;
	};

	template<typename T>
	class ComponentTypeDesc : public IComponentTypeDesc
	{
	public:
		~ComponentTypeDesc() override = default;

		void SetLabel(std::string_view name);
		void SetDescription(std::string_view description);
		void SetGUID(const VoltGUID& guid);
		inline void SetHidden() { m_isHidden = true; }

		[[nodiscard]] inline const VoltGUID& GetGUID() const override { return m_guid; }
		[[nodiscard]] inline const std::string_view GetLabel() const override { return m_componentLabel; }
		[[nodiscard]] inline const std::string_view GetDescription() const override { return m_componentDescription; }
		[[nodiscard]] inline const std::vector<ComponentMember>& GetMembers() const override { return m_members; }
		[[nodiscard]] inline const bool IsHidden() const override { return m_isHidden; }

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue)
		{
			return AddMember(memberPtr, name, label, description, defaultValue, AssetType::None);
		}

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue, AssetType assetType)
		{
			static ComponentMember* nullMember = nullptr;

			const ptrdiff_t offset = Utility::GetMemberOffset(memberPtr);
			const bool hasMemberWithOffset = FindMemberByOffset(offset) != nullptr;
			VT_CORE_ASSERT(!hasMemberWithOffset, "Member with offset has already been registered");
			if (hasMemberWithOffset)
			{
				return *nullMember;
			}

			const bool hasMemberWithName = FindMemberByName(name) != nullptr;
			VT_CORE_ASSERT(!hasMemberWithName, "Member with name has already been registered!");
			if (hasMemberWithName)
			{
				return *nullMember;
			}

			if constexpr (IsReflectedType<Type>())
			{
				m_members.emplace_back(offset, name, label, description, assetType, GetTypeDesc<Type>(), typeid(Type), CreateScope<DefaultValueType<DefaultValueT>>(defaultValue));
			}
			else
			{
				m_members.emplace_back(offset, name, label, description, assetType, nullptr, typeid(Type), CreateScope<DefaultValueType<DefaultValueT>>(defaultValue));
			}


			return m_members.back();
		}

	private:
		ComponentMember* FindMemberByOffset(const ptrdiff_t offset);
		ComponentMember* FindMemberByName(std::string_view name);

		std::vector<ComponentMember> m_members;

		VoltGUID m_guid = VoltGUID::Null();
		std::string m_componentLabel;
		std::string m_componentDescription;

		bool m_isHidden = false;

	};

	template<Enum T>
	class EnumTypeDesc : public IEnumTypeDesc
	{
	public:
		~EnumTypeDesc() override = default;

		void SetLabel(std::string_view name);
		void SetDescription(std::string_view description);
		void SetGUID(const VoltGUID& guid);
		void SetDefaultValue(const T& value);

		void AddConstant(const T& constant, std::string_view name, std::string_view label);

		[[nodiscard]] inline const VoltGUID& GetGUID() const override { return m_guid; }
		[[nodiscard]] inline const std::string_view GetLabel() const override { return m_enumLabel; }
		[[nodiscard]] inline const std::string_view GetDescription() const override { return m_enumDescription; }
		[[nodiscard]] inline const std::vector<EnumConstant>& GetConstants() const override { return m_constants; }

	private:
		VoltGUID m_guid = VoltGUID::Null();
		std::string m_enumLabel;
		std::string m_enumDescription;

		std::vector<EnumConstant> m_constants;
		T m_defaultValue = static_cast<T>(0);
	};

	template<typename T>
	inline void ComponentTypeDesc<T>::SetLabel(std::string_view name)
	{
		m_componentLabel = name;
	}

	template<typename T>
	inline void ComponentTypeDesc<T>::SetDescription(std::string_view description)
	{
		m_componentDescription = description;
	}

	template<typename T>
	inline void ComponentTypeDesc<T>::SetGUID(const VoltGUID& guid)
	{
		m_guid = guid;
	}

	template<typename T>
	inline ComponentMember* ComponentTypeDesc<T>::FindMemberByOffset(const ptrdiff_t offset)
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
	inline ComponentMember* ComponentTypeDesc<T>::FindMemberByName(std::string_view name)
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

	template<Enum T>
	inline void EnumTypeDesc<T>::SetLabel(std::string_view name)
	{
		m_enumLabel = name;
	}
	
	template<Enum T>
	inline void EnumTypeDesc<T>::SetDescription(std::string_view description)
	{
		m_enumDescription = description;
	}
	
	template<Enum T>
	inline void EnumTypeDesc<T>::SetGUID(const VoltGUID& guid)
	{
		m_guid = guid;
	}
	
	template<Enum T>
	inline void EnumTypeDesc<T>::SetDefaultValue(const T& value)
	{
		m_defaultValue = value;
	}
	
	template<Enum T>
	inline void EnumTypeDesc<T>::AddConstant(const T& constant, std::string_view name, std::string_view label)
	{
		m_constants.emplace_back(static_cast<int32_t>(constant), name, label);
	}

	// Helpers
	template<typename T> class TypeDesc<T, typename std::enable_if<std::is_enum<T>::value>::type> : public EnumTypeDesc<T>
	{ 
	public:
		~TypeDesc() override = default;
	};

	template<typename T> class TypeDesc<T, typename std::enable_if<std::is_class<T>::value>::type> : public ComponentTypeDesc<T>
	{ 
	public:
		~TypeDesc() override = default;
	};

	namespace Helpers
	{
		template<typename T> constexpr auto IsReflectedType(int) -> decltype(ReflectType(std::declval<TypeDesc<T>&>()), bool())
		{
			return true;
		}

		template<typename T> constexpr bool IsReflectedType(...)
		{
			return false;
		}
	}

	template<typename T> class TypeDescImpl : public TypeDesc<T>
	{
	public:
		~TypeDescImpl() override = default;

		inline TypeDescImpl()
		{
			ReflectType(static_cast<TypeDesc<T>&>(*this));
		}
	};

	template<typename T> constexpr bool IsReflectedType()
	{
		return Helpers::IsReflectedType<T>(0);
	}

	template<typename T> inline const TypeDesc<T>* GetTypeDesc()
	{
		static const TypeDescImpl<T> s_desc;
		return &s_desc;
	}

	template<typename T> inline VoltGUID GetTypeGUID()
	{
		static const TypeDescImpl<T> s_desc;
		return s_desc.GetGUID();
	}
}
