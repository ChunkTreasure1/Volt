#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Core/Buffer.h"

#include <Wire/Wire.h>

#include "Volt/Net/Replicated/ReplicationConditions.h"

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

	enum class MonoFieldType : uint32_t
	{
		Unknown = 0,
		Bool,
		Char,
		UChar,
		Short,
		UShort,
		Int,
		UInt,
		Int64,
		UInt64,
		Float,
		Double,
		Vector2,
		Vector3,
		Vector4,
		String,

		Quaternion,
		Color,
		Enum,

		Entity,

		Asset,
		Animation,
		Prefab,
		Scene,
		Mesh,
		Font,
		Material,
		Texture,
		PostProcessingMaterial,
		Video,
		AnimationGraph
	};

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
		MonoFieldType type;
		FieldAccessibility fieldAccessability = FieldAccessibility::None;
		MonoClassField* fieldPtr = nullptr;
		std::string enumName;
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

		template<typename T>
		void SetValue(const T& value, const size_t size, const MonoFieldType& type)
		{
			data.Allocate(size);
			data.Copy((void*)&value, size, 0);
			field.type = type;
		}

		template<>
		void SetValue(const std::string& value, const size_t, const MonoFieldType& type)
		{
			data.Allocate(value.size() + 1);
			data.Copy(value.c_str(), value.size() + 1);
			field.type = type;
		}
	};

	class MonoScriptClass
	{
	public:
		MonoScriptClass() = default;
		MonoScriptClass(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className, bool isEngineScript = false);

		MonoMethod* GetMethod(const std::string& name, int32_t paramCount);

		inline MonoClass* GetClass() const { return myMonoClass; }
		inline std::string GetNamespace() const { return myNamespace; }
		inline std::string GetClassName() const { return myClassName; }
		inline const bool IsEngineScript() const { return myIsEngineScript; }
		
		inline const std::unordered_map<std::string, MonoScriptField>& GetFields() const { return myFields; }

		bool IsSubclassOf(Ref<MonoScriptClass> parent);

		static MonoFieldType WirePropTypeToMonoFieldType(const Wire::ComponentRegistry::PropertyType& type);
		static Wire::ComponentRegistry::PropertyType MonoFieldTypeToWirePropType(const MonoFieldType& type);
		static bool IsAsset(const MonoFieldType& type);

	private:
		void FindAndCacheFields();
		void FindAndCacheSubClassFields(MonoClass* klass);

		MonoMethod* TryGetMethodOfParent(MonoClass* klass, const std::string& methodName, int32_t paramCount);

		MonoFieldType GetTypeFromString(const std::string& str);

		std::string myNamespace;
		std::string myClassName;

		MonoClass* myMonoClass = nullptr;
		std::unordered_map<std::string, MonoMethod*> myMethodCache;
		std::unordered_map<std::string, MonoScriptField> myFields;

		bool myIsEngineScript = false;

		uint32_t myHandle;
	};
}
