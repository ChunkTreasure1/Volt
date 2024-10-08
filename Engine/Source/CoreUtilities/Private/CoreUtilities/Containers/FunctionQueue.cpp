#include "cupch.h"
#include "Containers/FunctionQueue.h"

FunctionQueue::FunctionQueue(const FunctionQueue& other)
{
	m_queue = other.m_queue;
}

void FunctionQueue::Push(std::function<void()>&& function)
{
	std::scoped_lock lock{ m_queueMutex };
	m_queue.emplace_back(function);
}

void FunctionQueue::Flush()
{
	std::scoped_lock lock{ m_queueMutex };
	for (auto& func : m_queue)
	{
		func();
	}

	m_queue.clear();
}

void FunctionQueue::Clear()
{
	std::scoped_lock lock{ m_queueMutex };
	m_queue.clear();
}
