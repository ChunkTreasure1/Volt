#pragma once

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

#include <string>


namespace Volt::RHI
{
	enum class InputUsage
	{
		PerVertex,
		PerInstance
	};

	enum class ElementType : uint32_t
	{
		Bool = 0,

		Byte,
		Byte2,
		Byte3,
		Byte4,

		Half,
		Half2,
		Half3,
		Half4,

		UShort,
		UShort2,
		UShort3,
		UShort4,

		Int,
		Int2,
		Int3,
		Int4,

		UInt,
		UInt2,
		UInt3,
		UInt4,

		Float,
		Float2,
		Float3,
		Float4,

		Float3x3,
		Float4x4
	};

	struct BufferElement
	{
		BufferElement() = default;
		BufferElement(ElementType aElementType, const std::string& aName, uint32_t aArrayIndex = 0, InputUsage aUsage = InputUsage::PerVertex, uint32_t aInputSlot = 0)
			: name(aName), size(GetSizeFromType(aElementType)), arrayIndex(aArrayIndex), inputSlot(aInputSlot), type(aElementType), usage(aUsage)
		{
		}

		static uint32_t GetSizeFromType(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return 4; // HLSL bool size

				case ElementType::Byte: return 1;
				case ElementType::Byte2: return 2;
				case ElementType::Byte3: return 3;
				case ElementType::Byte4: return 4;

				case ElementType::Half: return 2;
				case ElementType::Half2: return 2 * 2;
				case ElementType::Half3: return 2 * 3;
				case ElementType::Half4: return 2 * 4;

				case ElementType::UShort: return 2;
				case ElementType::UShort2: return 2 * 2;
				case ElementType::UShort3: return 2 * 3;
				case ElementType::UShort4: return 2 * 4;

				case ElementType::Int: return 4;
				case ElementType::Int2: return 4 * 2;
				case ElementType::Int3: return 4 * 3;
				case ElementType::Int4: return 4 * 4;

				case ElementType::UInt: return 4;
				case ElementType::UInt2: return 4 * 2;
				case ElementType::UInt3: return 4 * 3;
				case ElementType::UInt4: return 4 * 4;

				case ElementType::Float: return 4;
				case ElementType::Float2: return 4 * 2;
				case ElementType::Float3: return 4 * 3;
				case ElementType::Float4: return 4 * 4;

				case ElementType::Float3x3: return 4 * 3 * 3;
				case ElementType::Float4x4: return 4 * 4 * 4;
			}

			return 0;
		}

		uint32_t GetComponentCount(ElementType elementType)
		{
			switch (elementType)
			{
				case ElementType::Bool: return 1;

				case ElementType::Byte: return 1;
				case ElementType::Byte2: return 2;
				case ElementType::Byte3: return 3;
				case ElementType::Byte4: return 4;

				case ElementType::Half: return 1;
				case ElementType::Half2: return 2;
				case ElementType::Half3: return 3;
				case ElementType::Half4: return 4;

				case ElementType::UShort: return 1;
				case ElementType::UShort2: return 2;
				case ElementType::UShort3: return 3;
				case ElementType::UShort4: return 4;

				case ElementType::Int: return 1;
				case ElementType::Int2: return 2;
				case ElementType::Int3: return 3;
				case ElementType::Int4: return 4;

				case ElementType::UInt: return 1;
				case ElementType::UInt2: return 2;
				case ElementType::UInt3: return 3;
				case ElementType::UInt4: return 4;

				case ElementType::Float: return 1;
				case ElementType::Float2: return 2;
				case ElementType::Float3: return 3;
				case ElementType::Float4: return 4;

				case ElementType::Float3x3: return 3 * 3;
				case ElementType::Float4x4: return 4 * 4;
			}

			return 0;
		}

		static void Serialize(BinaryStreamWriter& streamWriter, const BufferElement& data)
		{
			streamWriter.Write(data.name);
			streamWriter.Write(data.offset);
			streamWriter.Write(data.size);
			streamWriter.Write(data.arrayIndex);
			streamWriter.Write(data.inputSlot);
			streamWriter.Write(data.type);
			streamWriter.Write(data.usage);
		}

		static void Deserialize(BinaryStreamReader& streamReader, BufferElement& outData)
		{
			streamReader.Read(outData.name);
			streamReader.Read(outData.offset);
			streamReader.Read(outData.size);
			streamReader.Read(outData.arrayIndex);
			streamReader.Read(outData.inputSlot);
			streamReader.Read(outData.type);
			streamReader.Read(outData.usage);
		}

		std::string name;
		size_t offset;

		uint32_t size;
		uint32_t arrayIndex;
		uint32_t inputSlot;

		ElementType type;
		InputUsage usage;
	};

	class BufferLayout
	{
	public:
		BufferLayout()
			: m_stride(0)
		{
		}

		BufferLayout(std::initializer_list<BufferElement> aElements)
			: m_elements(aElements), m_stride(0)
		{
			CalculateOffsetAndStride();
		}

		BufferLayout(Vector<BufferElement> aElements)
			: m_elements(aElements), m_stride(0)
		{
			CalculateOffsetAndStride();
		}

		VT_NODISCARD VT_INLINE static std::string GetNameFromElementType(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return "Bool";

				case ElementType::Byte: return "Byte";
				case ElementType::Byte2: return "Byte2";
				case ElementType::Byte3: return "Byte3";
				case ElementType::Byte4: return "Byte4";

				case ElementType::Half: return "Half";
				case ElementType::Half2: return "Half2";
				case ElementType::Half3: return "Half3";
				case ElementType::Half4: return "Half4";

				case ElementType::UShort: return "UShort";
				case ElementType::UShort2: return "UShort2";
				case ElementType::UShort3: return "UShort3";
				case ElementType::UShort4: return "UShort4";

				case ElementType::Int: return "Int";
				case ElementType::Int2: return "Int2";
				case ElementType::Int3: return "Int3";
				case ElementType::Int4: return "Int4";

				case ElementType::UInt: return "UInt";
				case ElementType::UInt2: return "UInt2";
				case ElementType::UInt3: return "UInt3";
				case ElementType::UInt4: return "UInt4";

				case ElementType::Float: return "Float";
				case ElementType::Float2: return "Float2";
				case ElementType::Float3: return "Float3";
				case ElementType::Float4: return "Float4";

				case ElementType::Float3x3: return "Float3x3";
				case ElementType::Float4x4: return "Float4x4";
			}

			return "None";
		}

		VT_NODISCARD VT_INLINE const uint32_t GetStride() const { return m_stride; }
		VT_NODISCARD VT_INLINE const Vector<BufferElement>& GetElements() const { return m_elements; }
		VT_NODISCARD VT_INLINE const bool IsValid() const { return !m_elements.empty(); }

		static void Serialize(BinaryStreamWriter& streamWriter, const BufferLayout& data)
		{
			streamWriter.Write(data.m_elements);
			streamWriter.Write(data.m_stride);
		}

		static void Deserialize(BinaryStreamReader& streamReader, BufferLayout& outData)
		{
			streamReader.Read(outData.m_elements);
			streamReader.Read(outData.m_stride);

			outData.CalculateOffsetAndStride();
		}

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			uint32_t lastInputSlot = 0;
			m_stride = 0;

			for (auto& element : m_elements)
			{
				if (lastInputSlot != element.inputSlot)
				{
					lastInputSlot = element.inputSlot;
					offset = 0;
				}
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		Vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
	};
}
