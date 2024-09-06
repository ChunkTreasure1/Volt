#pragma once 
#include <deque>
#include <mutex>

template<typename TYPE>
class tsdeque
{
public:
	tsdeque() = default;
	tsdeque(const tsdeque<TYPE>&) = delete;
	virtual	~tsdeque() { clear(); }

	const TYPE& front()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		return deqQueue.front();
	}
	const TYPE& back()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		return deqQueue.back();
	}
	const void push_back(const TYPE& item)
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.emplace_back(std::move(item));
	}
	const void push_front(const TYPE& item)
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.emplace_front(std::move(item));
	}
	bool empty()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		return deqQueue.empty();
	}
	size_t count()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		return deqQueue.size();
	}
	void clear()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.clear();
	}
	void wait()
	{
		while (empty())
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.wait(ul);
		}
	}
	TYPE pop_front()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		auto t = std::move(deqQueue.front());
		deqQueue.pop_front();
		return t;
	}
	TYPE pop_back()
	{
		std::lock_guard<std::mutex> lock(muxQueue);
		auto t = std::move(deqQueue.front());
		deqQueue.pop_back();
		return t;
	}

protected:
	std::mutex muxQueue;
	std::deque<TYPE> deqQueue;
	std::mutex muxBlocking;
	std::condition_variable cvBlocking;
};
