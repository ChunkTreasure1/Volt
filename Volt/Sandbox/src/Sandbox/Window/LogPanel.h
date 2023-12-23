#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Log/CallbackSink.h>

class LogPanel : public EditorWindow
{
public:
	LogPanel();
	~LogPanel() override = default;

	void UpdateMainContent() override;

private:
	uint32_t myMaxMessages = 1000;
	std::vector<LogCallbackData> myLogMessages;
	std::vector<LogCallbackData> myCurrentLogMessages;

	std::vector<std::string> myCategories;

	std::mutex m_logMutex;
};
