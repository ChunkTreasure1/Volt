#pragma once

#include "CoreUtilities/Containers/Vector.h"

#include <functional>

class VTCOREUTIL_API FunctionQueue
{
public:
	FunctionQueue() = default;
	FunctionQueue(const FunctionQueue& other);

	void Push(std::function<void()>&& function);
	void Flush();
	void Clear();

private:
	Vector<std::function<void()>> m_queue;
	std::mutex m_queueMutex;
};
