#pragma once

namespace Wire
{
	struct DestructorBase
	{
		virtual void Destroy(void* ptr) = 0;
		virtual ~DestructorBase() = default;
	};

	template<typename T>
	struct Destructor : public DestructorBase
	{
		void Destroy(void* ptr) override
		{
			((T*)ptr)->~T();
		}
	};
}