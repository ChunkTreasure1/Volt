#pragma once

#include "Volt/Core/Buffer.h"

namespace Volt
{
	class BinarySerializer
	{
	public:
		BinarySerializer(const std::filesystem::path& targetFile, const size_t maxSize = 0);
		~BinarySerializer();

		template<typename T>
		void Serialize(const T& value);
		
		void Serialize(const void* data, const size_t size);

		void WriteToFile();

	private:
		size_t myMaxSize = 0;
		size_t myCurrentSize = 0;

		Buffer myDataBuffer;
		std::filesystem::path myTargetPath;
	};

	template<typename T>
	inline void BinarySerializer::Serialize(const T& value)
	{
		Serialize(&value, sizeof(T));
	}

	template<>
	inline void BinarySerializer::Serialize<std::string>(const std::string& value)
	{
		Serialize(value.data(), value.size());
	}
}