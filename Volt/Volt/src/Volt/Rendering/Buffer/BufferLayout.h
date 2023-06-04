#pragma once

#include <string>
#include <vector>

namespace Volt
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
		BufferElement(ElementType aElementType, const std::string& aName, uint32_t aArrayIndex = 0, InputUsage aUsage = InputUsage::PerVertex, uint32_t aInputSlot = 0)
			: type(aElementType), name(aName), size(GetSizeFromType(aElementType)), arrayIndex(aArrayIndex), usage(aUsage), inputSlot(aInputSlot)
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
			: myStride(0)
		{
		}

		BufferLayout(std::initializer_list<BufferElement> aElements)
			: myElements(aElements), myStride(0)
		{
			CalculateOffsetAndStride();
		}

		BufferLayout(std::vector<BufferElement> aElements)
			: myElements(aElements), myStride(0)
		{
			CalculateOffsetAndStride();
		}

		inline const uint32_t GetStride() const { return myStride; }
		inline std::vector<BufferElement>& GetElements() { return myElements; }
		inline const bool IsValid() const { return !myElements.empty(); }

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			uint32_t lastInputSlot = 0;
			myStride = 0;

			for (auto& element : myElements)
			{
				if (lastInputSlot != element.inputSlot)
				{
					lastInputSlot = element.inputSlot;
					offset = 0;
				}
				element.offset = offset;
				offset += element.size;
				myStride += element.size;
			}
		}

		std::vector<BufferElement> myElements;
		uint32_t myStride = 0;
	};
}
