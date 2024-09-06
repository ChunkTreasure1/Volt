#pragma once

#include "Sandbox/Window/EditorWindow.h"

class LogPanel : public EditorWindow
{
public:
	LogPanel();
	~LogPanel() override;

	void UpdateMainContent() override;

private:
	uint32_t m_maxMessages = 1000;
	Vector<LogCallbackData> m_logMessages;
	Vector<LogCallbackData> m_currentLogMessages;

	Vector<std::string> m_categories;

	LogCallbackHandle m_callbackHandle = 0;
	std::mutex m_logMutex;
};
