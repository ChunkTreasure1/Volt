#pragma once

#include "Sandbox/Window/EditorWindow.h"

class LogPanel : public EditorWindow
{
public:
	LogPanel();
	~LogPanel() override = default;

	void UpdateMainContent() override;

private:
	uint32_t m_maxMessages = 1000;
	Vector<LogCallbackData> m_logMessages;
	Vector<LogCallbackData> m_currentLogMessages;

	Vector<std::string> m_categories;

	std::mutex m_logMutex;
};
