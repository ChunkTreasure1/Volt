#pragma once

#include <spdlog/sinks/base_sink.h>
#include <vector>
#include <functional>

struct LogCallbackData
{
	std::string message;
	spdlog::level::level_enum level;
	std::string category;
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
		std::string category = "Default";

		std::string text = fmt::to_string(formatted);
		
		auto startOfCat = text.find("$[");
		if (startOfCat != std::string::npos)
		{
			std::string msg = text.substr(startOfCat, text.size());
			auto endOfCat = msg.find(']');
			if (endOfCat != std::string::npos)
			{
				category = msg.substr(2, endOfCat - 2);

				auto pieceOne = text.substr(0, startOfCat);
				auto pieceTwo = text.substr(startOfCat + endOfCat + 1, text.size());
				text = pieceOne + pieceTwo;
			}
		}

		for (const auto& callback : m_callbacks)
		{
			callback({ text, msg.level, category});
		}
	}

	void flush_() override
	{
	}

	std::vector<std::function<void(const LogCallbackData&)>> m_callbacks;
};
