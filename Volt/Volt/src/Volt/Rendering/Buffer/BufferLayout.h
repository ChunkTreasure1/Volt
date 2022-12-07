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
		Int,	

		UInt,
		UInt2,
		UInt3,
		UInt4,

		Float,
		Float2,
		Float3,
		Float4,

		Mat3,
		Mat4
	};

	struct BufferElement
	{
		BufferElement(ElementType aElementType, const std::string& aName, uint32_t aArrayIndex = 0, InputUsage aUsage = InputUsage::PerVertex, uint32_t aInputSlot = 0)
			: type(aElementType), name(aName), size(GetSizeFromType(aElementType)), arrayIndex(aArrayIndex), usage(aUsage), inputSlot(aInputSlot)
		{}

		static uint32_t GetSizeFromType(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return 4; // HLSL bool size
				case ElementType::Int: return 4;
				case ElementType::UInt: return 4;
				case ElementType::UInt2: return 4 * 2;
				case ElementType::UInt3: return 4 * 3;
				case ElementType::UInt4: return 4 * 4;
				case ElementType::Float: return 4;
				case ElementType::Float2: return 4 * 2;
				case ElementType::Float3: return 4 * 3;
				case ElementType::Float4: return 4 * 4;
				case ElementType::Mat3: return 4 * 3 * 3;
				case ElementType::Mat4: return 4 * 4 * 4;
			}

			return 0;
		}

		uint32_t GetComponentCount(ElementType elementType)
		{
			switch (elementType)
			{
				case ElementType::Bool: return 1;
				case ElementType::Int: return 1;

				case ElementType::UInt: return 1;
				case ElementType::UInt2: return 2;
				case ElementType::UInt3: return 3;
				case ElementType::UInt4: return 4;

				case ElementType::Float: return 1;
				case ElementType::Float2: return 2;
				case ElementType::Float3: return 3;
				case ElementType::Float4: return 4;

				case ElementType::Mat3: return 3 * 3;
				case ElementType::Mat4: return 4 * 4;
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
		{}

		BufferLayout(std::initializer_list<BufferElement> aElements)
			: myElements(aElements), myStride(0)
		{
			CalculateOffsetAndStride();
		}

		inline const uint32_t GetStride() const { return myStride; }
		inline std::vector<BufferElement>& GetElements() { return myElements; }

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