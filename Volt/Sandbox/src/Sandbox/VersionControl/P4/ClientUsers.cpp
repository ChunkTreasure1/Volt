#include "sbpch.h"
#include "ClientUsers.h"

void StreamsClientUser::OutputInfo(char level, const char* data)
{
	std::string dataStr(data);

	if (dataStr.find("Stream ") != std::string::npos)
	{
		std::string streamName = dataStr.substr(dataStr.find_first_of(' ') + 1);
		m_streams.emplace_back(streamName);
	}
}

void StreamsClientUser::Clear()
{
	m_streams.clear();
}

void WorkspacesClientUser::OutputInfo(char level, const char* data)
{
	std::string dataStr(data);

	if (dataStr.find("client ") != std::string::npos)
	{
		std::string clientName = dataStr.substr(dataStr.find_first_of(' ') + 1);
		m_workspaces.emplace_back(clientName);
	}
}

void WorkspacesClientUser::Clear()
{
	m_workspaces.clear();
}
