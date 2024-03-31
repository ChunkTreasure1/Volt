#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Buffer.h"

#include "Volt/Net/Replicated/ReplicationConditions.h"

#include "Volt/Scripting/Mono/MonoTypeRegistry.h"

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Volt
{
	// **Note**
	// When adding new field types make sure to update the following classes if needed.
	// --------
	// Entity::Copy
	// MonoScriptClass::IsAsset
	// MonoScriptClass::GetTypeFromString
	// SceneImporter::DeserializeMono
	// PropertiesPanel::DrawMonoProperties
	// PropertiesPanel::AssetTypeFromMonoType
	// MonoScriptEngine::SetScriptFieldDefaultData
	// --------

	enum class FieldAccessibility : uint8_t
	{
		None = 0,
		Private = (1 << 0),
		Internal = (1 << 1),
		Protected = (1 << 2),
		Public = (1 << 3),
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(FieldAccessibility);

	struct MonoFieldNetAttribute
	{
		eRepCondition replicatedCondition = eRepCondition::OFF;
		std::string boundFunction = "";
	};

	struct MonoScriptField
	{
		MonoTypeInfo type;
		FieldAccessibility fieldAccessability = FieldAccessibility::None;
		MonoClassField* fieldPtr = nullptr;
		MonoFieldNetAttribute netData;
	};

	struct MonoScriptFieldInstance
	{
		~MonoScriptFieldInstance()
		{
			data.Release();
		}

		MonoScriptField field;
		Buffer data;

		inline void Reset(const size_t size)
		{
			data.Allocate(size);
		}

		inline void SetValue(const void* value, const size_t size)
		{
			data.Allocate(size);
			data.Copy(value, size);
		}

		template<typename T>
		inline void SetValue(const T& value, const size_t size)
		{
			data.Allocate(size);
			data.Copy((void*)&value, size, 0);
		}

		template<>
		inline void SetValue(const std::string& value, const size_t)
		{
			data.Allocate(value.size() + 1);
			data.Copy(value.c_str(), value.size() + 1);
		}
	};

	class MonoScriptClass
	{
	public:
		MonoScriptClass() = default;
		MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className, bool isEngineScript = false);

		MonoMethod* GetMethod(const std::string& name, int32_t paramCount);

		inline MonoClass* GetClass() const { return m_monoClass; }
		inline std::string GetNamespace() const { return m_namespace; }
		inline std::string GetClassName() const { return m_className; }
		inline const bool IsEngineScript() const { return m_isEngineScript; }
		
		inline const std::unordered_map<std::string, MonoScriptField>& GetFields() const { return m_fields; }

		bool IsSubclassOf(Ref<MonoScriptClass> parent);

	private:
		void FindAndCacheFields();
		void FindAndCacheSubClassFields(MonoClass* klass);

		MonoMethod* TryGetMethodOfParent(MonoClass* klass, const std::string& methodName, int32_t paramCount);

		std::string m_namespace;
		std::string m_className;

		MonoClass* m_monoClass = nullptr;
		std::unordered_map<std::string, MonoMethod*> m_methodCache;
		std::unordered_map<std::string, MonoScriptField> m_fields;

		bool m_isEngineScript = false;

		uint32_t m_handle;
	};
}
