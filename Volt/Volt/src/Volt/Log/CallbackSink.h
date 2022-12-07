#pragma once

#include <spdlog/sinks/base_sink.h>
#include <vector>
#include <functional>

struct LogCallbackData
{
	std::string message;
	spdlog::level::level_enum level;
};

template<typename Mutex>
class CallbackSink : public spdlog::sinks::base_sink<Mutex>
{
public:
	CallbackSink() = default;
	explicit CallbackSink(std::unique_ptr<spdlog::formatter> formatter)
	{
	}

	void AddCallback(std::function<void(const LogCallbackData&)> function)
	{
		m_callbacks.push_back(function);
	}

	void ClearCallbacks()
	{
		m_callbacks.clear();
	}

private:
	void sink_it_(const spdlog::details::log_msg& msg)
	{
		spdlog::memory_buf_t formatted;
		spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

		for (const auto& callback : m_callbacks)
		{
			callback({ fmt::to_string(formatted), msg.level });
		}
	}

	void flush_() override
	{
	}

	std::vector<std::function<void(const LogCallbackData&)>> m_callbacks;
};