#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include "Volt/Utility/MemoryUtility.h"
#include "Volt/Asset/Asset.h"

#include "Volt/Scene/Entity.h"

#include <CoreUtilities/VoltGUID.h>

#include <typeindex>
#include <vector>

#define DECLARE_ARRAY_TYPE(type)											\
namespace Volt																\
{																			\
	template<typename ELEMENT_TYPE> struct ArrayTraits<type<ELEMENT_TYPE>>  \
	{																		\
		typedef ELEMENT_TYPE element_type;									\
																			\
		static const bool is_array = true;									\
	};																		\
}																			\

namespace Volt
{
	template<typename T>
	concept Enum = std::is_enum<T>::value;

	template<typename T, class ENABLE = void> class TypeDesc;
	template<typename T> constexpr bool IsReflectedType();
	template<typename T> constexpr bool IsArrayType();

	template<typename T> const TypeDesc<T>* GetTypeDesc();

	template<typename T> inline constexpr auto ReflectType(TypeDesc<T>& desc) -> decltype(T::ReflectType(std::declval<TypeDesc<T>&>()), void())
	{
		T::ReflectType(desc);
	}

	enum class ValueType
	{
		Default,
		Component,
		Enum,
		Array
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

	enum class ComponentMemberFlag
	{
		None = 0,
		Color3 = BIT(0),
		Color4 = BIT(1),

		NoCopy = BIT(2),
		NoSerialize = BIT(3)
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ComponentMemberFlag);

	struct ComponentMember
	{
		ptrdiff_t offset;
		std::string_view name;
		std::string_view label;
		std::string_view description;

		AssetType assetType = AssetType::None;
		ComponentMemberFlag flags = ComponentMemberFlag::None;

		const ICommonTypeDesc* typeDesc = nullptr;
		const ICommonTypeDesc* ownerTypeDesc = nullptr;

		std::type_index typeIndex = typeid(void);
		Scope<IDefaultValueType> defaultValue;

		std::function<void(void* lhs, const void* rhs)> copyFunction;
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
		[[nodiscard]] virtual ComponentMember* FindMemberByOffset(const ptrdiff_t offset) = 0;
		[[nodiscard]] virtual ComponentMember* FindMemberByName(std::string_view name) = 0;
		[[nodiscard]] virtual const ComponentMember* FindMemberByName(std::string_view name) const = 0;
		
		virtual void OnMemberChanged(void* objectPtr, Entity entity) const = 0;
	};

	class IEnumTypeDesc : public CommonTypeDesc<ValueType::Enum>
	{
	public:
		~IEnumTypeDesc() override = default;

		[[nodiscard]] virtual const std::vector<EnumConstant>& GetConstants() const = 0;
		[[nodiscard]] virtual const std::vector<std::string> GetConstantNames() const = 0;
	};

	class IArrayTypeDesc : public CommonTypeDesc<ValueType::Array>
	{
	public:
		[[nodiscard]] virtual const ICommonTypeDesc* GetElementTypeDesc() const = 0;
		[[nodiscard]] virtual const std::type_index& GetElementTypeIndex() const = 0;
		[[nodiscard]] virtual const size_t GetElementTypeSize() const = 0;

		// NOTE: This function heap allocates an object that MUST be manually deleted!
		virtual void DefaultConstructElement(void*& value) const = 0;
		virtual void DestroyElement(void*& value) const = 0;

		virtual void PushBack(void* array, const void* value) const = 0;
		[[nodiscard]] virtual const size_t Size(const void* array) const = 0;
		[[nodiscard]] virtual void* At(void* array, size_t pos) const = 0;
		[[nodiscard]] virtual const void* At(const void* array, size_t pos) const = 0;
		virtual void* EmplaceBack(void* array, const void* value) const = 0;
		virtual void Erase(void* array, size_t pos) const = 0;

		~IArrayTypeDesc() override = default;
	};

	template<typename T, typename ELEMENT_TYPE>
	class ArrayTypeDesc : public IArrayTypeDesc
	{
	public:
		~ArrayTypeDesc() override = default;

		void SetLabel(std::string_view name);
		void SetDescription(std::string_view description);
		void SetGUID(const VoltGUID& guid);

		void SetSizeFunction(std::function<size_t(const void*)>&& func)
		{
			m_sizeFunction = std::move(func);
		}

		void SetAtFunction(std::function<void* (void*, size_t)>&& func)
		{
			m_atFunction = std::move(func);
		}

		void SetConstAtFunction(std::function<const void* (const void*, size_t)>&& func)
		{
			m_atConstFunction = std::move(func);
		}

		void SetPushBackFunction(std::function<void(void*, const void*)>&& func)
		{
			m_pushBackFunction = std::move(func);
		}

		void SetEmplaceBackFunction(std::function<void* (void*, const void*)>&& func)
		{
			m_emplaceBackFunction = std::move(func);
		}

		void SetEraseFunction(std::function<void(void*, size_t)>&& func)
		{
			m_eraseFunction = std::move(func);
		}

		[[nodiscard]] const size_t Size(const void* array) const override
		{
			return m_sizeFunction(array);
		}

		[[nodiscard]] void* At(void* array, size_t pos) const override
		{
			return m_atFunction(array, pos);
		}

		[[nodiscard]] const void* At(const void* array, size_t pos) const override
		{
			return m_atConstFunction(array, pos);
		}

		void PushBack(void* array, const void* value) const override
		{
			m_pushBackFunction(array, value);
		}

		void* EmplaceBack(void* array, const void* value) const override
		{
			return m_emplaceBackFunction(array, value);
		}

		void Erase(void* array, size_t pos) const override
		{
			m_eraseFunction(array, pos);
		}

		void DefaultConstructElement(void*& value) const override
		{
			value = new ELEMENT_TYPE();
		}

		void DestroyElement(void*& value) const override
		{
			ELEMENT_TYPE* typePtr = reinterpret_cast<ELEMENT_TYPE*>(value);
			delete typePtr;

			value = nullptr;
		}

		[[nodiscard]] inline const VoltGUID& GetGUID() const override { return m_guid; }
		[[nodiscard]] inline const std::string_view GetLabel() const override { return m_label; }
		[[nodiscard]] inline const std::string_view GetDescription() const override { return m_description; }
		[[nodiscard]] inline const size_t GetElementTypeSize() const override { return sizeof(ELEMENT_TYPE); }
		[[nodiscard]] inline const ICommonTypeDesc* GetElementTypeDesc() const override
		{
			if constexpr (IsReflectedType<ELEMENT_TYPE>() || IsArrayType<ELEMENT_TYPE>())
			{
				return GetTypeDesc<ELEMENT_TYPE>();
			}
			else
			{
				return nullptr;
			}
		}

		[[nodiscard]] inline const std::type_index& GetElementTypeIndex() const override { return m_elementTypeIndex; }

	private:
		VoltGUID m_guid = VoltGUID::Null();
		std::string m_label;
		std::string m_description;

		std::type_index m_elementTypeIndex = { typeid(ELEMENT_TYPE) };

		std::function<size_t(const void* pArray)> m_sizeFunction;
		std::function<void* (void* pArray, size_t pos)> m_atFunction;
		std::function<const void* (const void* pArray, size_t pos)> m_atConstFunction;
		std::function<void(void* pArray, const void* pValue)> m_pushBackFunction;
		std::function<void* (void* pArray, const void* pValue)> m_emplaceBackFunction;
		std::function<void(void* pArray, size_t pos)> m_eraseFunction;
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

		void OnMemberChanged(void* objectPtr, Entity entity) const override;

		[[nodiscard]] ComponentMember* FindMemberByOffset(const ptrdiff_t offset) override;
		[[nodiscard]] ComponentMember* FindMemberByName(std::string_view name) override;
		[[nodiscard]] const ComponentMember* FindMemberByName(std::string_view name) const override;

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue)
		{
			return AddMember(memberPtr, name, label, description, defaultValue, AssetType::None);
		}

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue, ComponentMemberFlag flags)
		{
			return AddMember(memberPtr, name, label, description, defaultValue, AssetType::None, flags);
		}

		template<typename Type, typename DefaultValueT, typename TypeParent = T>
		const ComponentMember& AddMember(Type TypeParent::* memberPtr, std::string_view name, std::string_view label, std::string_view description, const DefaultValueT& defaultValue, AssetType assetType, ComponentMemberFlag flags = ComponentMemberFlag::None)
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

			auto copyFunction = [](void* lhs, const void* rhs)
			{
				*reinterpret_cast<Type*>(lhs) = *reinterpret_cast<const Type*>(rhs);
			};

			if constexpr (IsArrayType<Type>() || IsReflectedType<Type>())
			{
				m_members.emplace_back(offset, name, label, description, assetType, flags, GetTypeDesc<Type>(), this, typeid(Type), CreateScope<DefaultValueType<DefaultValueT>>(defaultValue), copyFunction);
			}
			else
			{
				m_members.emplace_back(offset, name, label, description, assetType, flags, nullptr, this, typeid(Type), CreateScope<DefaultValueType<DefaultValueT>>(defaultValue), copyFunction);
			}


			return m_members.back();
		}

		void SetOnMemberChangedCallback(std::function<void(T&, Entity)>&& func)
		{
			m_onMemberChangedCallback = std::move(func);
		}

	private:
		std::vector<ComponentMember> m_members;

		VoltGUID m_guid = VoltGUID::Null();
		std::string m_componentLabel;
		std::string m_componentDescription;

		bool m_isHidden = false;
		std::function<void(T& component, Entity entity)> m_onMemberChangedCallback;
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
		[[nodiscard]] const std::vector<std::string> GetConstantNames() const override;

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
	inline void ComponentTypeDesc<T>::OnMemberChanged(void* objectPtr, Entity entity) const
	{
		if (m_onMemberChangedCallback)
		{
			m_onMemberChangedCallback(*reinterpret_cast<T*>(objectPtr), entity);
		}
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

	template<typename T>
	inline const ComponentMember* ComponentTypeDesc<T>::FindMemberByName(std::string_view name) const
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

	template<Enum T>
	inline const std::vector<std::string> EnumTypeDesc<T>::GetConstantNames() const
	{
		std::vector<std::string> result{};

		for (const auto& constant : m_constants)
		{
			result.emplace_back(std::string(constant.label));
		}

		return result;
	}

	// Helpers
	template<typename T> struct ArrayTraits
	{
		typedef void element_type;
		static const bool is_array = false;
	};

	template<typename T> struct IsClassType
	{
		static const bool value = std::is_class<T>::value && !std::is_enum<T>::value && !ArrayTraits<T>::is_array;
	};

	template<typename T> class TypeDesc<T, typename std::enable_if<std::is_enum<T>::value>::type> : public EnumTypeDesc<T>
	{
	public:
		~TypeDesc() override = default;
	};

	template<typename T> class TypeDesc<T, typename std::enable_if<IsClassType<T>::value>::type> : public ComponentTypeDesc<T>
	{
	public:
		~TypeDesc() override = default;
	};

	template<typename T> class TypeDesc<T, typename std::enable_if<ArrayTraits<T>::is_array>::type> : public ArrayTypeDesc<T, typename ArrayTraits<T>::element_type>
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

	template<typename T>
	constexpr bool IsArrayType()
	{
		return ArrayTraits<T>::is_array;
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

	template<typename T, typename ELEMENT_TYPE>
	inline void ArrayTypeDesc<T, ELEMENT_TYPE>::SetLabel(std::string_view name)
	{
		m_label = name;
	}

	template<typename T, typename ELEMENT_TYPE>
	inline void ArrayTypeDesc<T, ELEMENT_TYPE>::SetDescription(std::string_view description)
	{
		m_description = description;
	}

	template<typename T, typename ELEMENT_TYPE>
	inline void ArrayTypeDesc<T, ELEMENT_TYPE>::SetGUID(const VoltGUID& guid)
	{
		m_guid = guid;
	}
}

DECLARE_ARRAY_TYPE(std::vector);

namespace Volt
{
	template<typename ELEMENT_TYPE> inline void ReflectType(TypeDesc<std::vector<ELEMENT_TYPE>>& desc)
	{
		desc.SetLabel("std::vector");
		desc.SetSizeFunction([](const void* array) -> size_t
		{
			const std::vector<ELEMENT_TYPE>& ptr = *reinterpret_cast<const std::vector<ELEMENT_TYPE>*>(array);
			return ptr.size();
		});

		desc.SetAtFunction([](void* array, size_t pos) -> void*
		{
			return &(*reinterpret_cast<std::vector<ELEMENT_TYPE>*>(array))[pos];
		});

		desc.SetConstAtFunction([](const void* array, size_t pos) -> const void*
		{
			return &(*reinterpret_cast<const std::vector<ELEMENT_TYPE>*>(array)).at(pos);
		});

		desc.SetPushBackFunction([](void* array, const void* value) -> void
		{
			(*reinterpret_cast<std::vector<ELEMENT_TYPE>*>(array)).push_back(*reinterpret_cast<const ELEMENT_TYPE*>(value));
		});

		desc.SetEmplaceBackFunction([](void* array, const void* value) -> void*
		{
			if (!value)
			{
				return &(*reinterpret_cast<std::vector<ELEMENT_TYPE>*>(array)).emplace_back();
			}
			else
			{
				return &(*reinterpret_cast<std::vector<ELEMENT_TYPE>*>(array)).emplace_back(*reinterpret_cast<const ELEMENT_TYPE*>(value));
			}
		});

		desc.SetEraseFunction([](void* array, size_t pos) 
		{
			std::vector<ELEMENT_TYPE>& vecRef = *reinterpret_cast<std::vector<ELEMENT_TYPE>*>(array);
			vecRef.erase(vecRef.begin() + pos);
		});
	}
}
