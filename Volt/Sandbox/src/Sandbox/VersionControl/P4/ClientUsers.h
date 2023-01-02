#pragma once

#include <p4/clientapi.h>

#include <vector>
#include <string>

class StreamsClientUser : public ClientUser
{ 
public:
	void OutputInfo(char level, const char* data) override;
	void Clear();

	inline const std::vector<std::string>& GetData() const { return m_streams; }

private:
	std::vector<std::string> m_streams;
};

class WorkspacesClientUser : public ClientUser
{
public:
	void OutputInfo(char level, const char* data) override;
	void Clear();

	inline const std::vector<std::string>& GetData() const { return m_workspaces; }

private:
	std::vector<std::string> m_workspaces;
};