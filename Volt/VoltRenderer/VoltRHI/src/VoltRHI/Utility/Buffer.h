#pragma once

#include <memory>

namespace Volt::RHI
{
	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(const Buffer& buffer);

		Buffer(size_t size);
		Buffer(const void* data, size_t size);
		~Buffer();

		void Release();
		void Resize(size_t size);
		void SetData(const void* data, size_t size, size_t offset = 0);

		[[nodiscard]] const bool IsValid() const;
		[[nodiscard]] const size_t GetSize() const;
		[[nodiscard]] void* GetData() const;

		Buffer& operator=(const Buffer& buffer);

		template<typename T>
		inline T* As(size_t offset = 0) const;

	private:
		uint8_t* m_data = nullptr;
		size_t m_size = 0;
	};

	template<typename T>
	inline T* Buffer::As(size_t offset) const
	{
		return reinterpret_cast<T*>(m_data + offset);
	}
}